#define _POSIX_C_SOURCE 200809L
#include "hash.h"
#include "secret.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

/* Compact public-domain SHA-256 (FIPS 180-4) */

#define ROTR(x,n) (((x)>>(n))|((x)<<(32-(n))))
#define CH(x,y,z)   (((x)&(y))^(~(x)&(z)))
#define MAJ(x,y,z)  (((x)&(y))^((x)&(z))^((y)&(z)))
#define EP0(x)  (ROTR(x,2)  ^ ROTR(x,13) ^ ROTR(x,22))
#define EP1(x)  (ROTR(x,6)  ^ ROTR(x,11) ^ ROTR(x,25))
#define SIG0(x) (ROTR(x,7)  ^ ROTR(x,18) ^ ((x)>>3))
#define SIG1(x) (ROTR(x,17) ^ ROTR(x,19) ^ ((x)>>10))

static const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

typedef struct {
    uint8_t  buf[64];
    uint32_t len;
    uint64_t bits;
    uint32_t h[8];
} Sha256;

static void sha256_compress(Sha256 *s, const uint8_t *blk) {
    uint32_t a = s->h[0], b = s->h[1], c = s->h[2], d = s->h[3];
    uint32_t e = s->h[4], f = s->h[5], g = s->h[6], hh = s->h[7];
    uint32_t w[64], t1, t2, i;
    for (i = 0; i < 16; i++)
        w[i] = ((uint32_t)blk[i*4]<<24)|((uint32_t)blk[i*4+1]<<16)|
               ((uint32_t)blk[i*4+2]<<8)|(uint32_t)blk[i*4+3];
    for (i = 16; i < 64; i++)
        w[i] = SIG1(w[i-2]) + w[i-7] + SIG0(w[i-15]) + w[i-16];
    for (i = 0; i < 64; i++) {
        t1 = hh + EP1(e) + CH(e,f,g) + K[i] + w[i];
        t2 = EP0(a) + MAJ(a,b,c);
        hh=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    s->h[0]+=a; s->h[1]+=b; s->h[2]+=c; s->h[3]+=d;
    s->h[4]+=e; s->h[5]+=f; s->h[6]+=g; s->h[7]+=hh;
}

static void sha256_init(Sha256 *s) {
    s->len = 0; s->bits = 0;
    s->h[0]=0x6a09e667; s->h[1]=0xbb67ae85; s->h[2]=0x3c6ef372; s->h[3]=0xa54ff53a;
    s->h[4]=0x510e527f; s->h[5]=0x9b05688c; s->h[6]=0x1f83d9ab; s->h[7]=0x5be0cd19;
}

static void sha256_feed(Sha256 *s, const void *data, size_t n) {
    const uint8_t *p = (const uint8_t *)data;
    for (size_t i = 0; i < n; i++) {
        s->buf[s->len++] = p[i];
        if (s->len == 64) { sha256_compress(s, s->buf); s->bits += 512; s->len = 0; }
    }
}

static void sha256_done(Sha256 *s, uint8_t out[32]) {
    uint32_t i = s->len;
    s->buf[i++] = 0x80;
    if (s->len < 56) {
        while (i < 56) s->buf[i++] = 0;
    } else {
        while (i < 64) s->buf[i++] = 0;
        sha256_compress(s, s->buf);
        memset(s->buf, 0, 56);
    }
    uint64_t total = s->bits + (uint64_t)s->len * 8;
    for (int j = 7; j >= 0; j--) { s->buf[56+j] = (uint8_t)total; total >>= 8; }
    sha256_compress(s, s->buf);
    for (uint32_t k = 0; k < 8; k++) {
        out[k*4]   = (uint8_t)(s->h[k] >> 24);
        out[k*4+1] = (uint8_t)(s->h[k] >> 16);
        out[k*4+2] = (uint8_t)(s->h[k] >>  8);
        out[k*4+3] = (uint8_t)(s->h[k]);
    }
}

void hash_visitor_id(const char *ip, const char *ua, char out[65]) {
    char date[11];
    time_t now = time(NULL);
    struct tm tm_buf;
    gmtime_r(&now, &tm_buf);
    strftime(date, sizeof(date), "%Y-%m-%d", &tm_buf);

    Sha256 s;
    sha256_init(&s);
    if (ip && *ip) sha256_feed(&s, ip, strlen(ip));
    sha256_feed(&s, "|", 1);
    if (ua && *ua) sha256_feed(&s, ua, strlen(ua));
    sha256_feed(&s, "|", 1);
    const char *sec = secret_get();
    sha256_feed(&s, sec, strlen(sec));
    sha256_feed(&s, "|", 1);
    sha256_feed(&s, date, strlen(date));

    uint8_t digest[32];
    sha256_done(&s, digest);

    for (int i = 0; i < 32; i++)
        snprintf(out + i*2, 3, "%02x", digest[i]);
    out[64] = '\0';
}
