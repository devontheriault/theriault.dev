#include "secret.h"
#include <stdio.h>
#include <string.h>

static char g_secret[65] = {0};

const char *secret_get(void) { return g_secret; }

int secret_init(void) {
    FILE *f = fopen("data/secret.key", "r");
    if (f) {
        if (fgets(g_secret, sizeof(g_secret), f))
            g_secret[strcspn(g_secret, "\n\r")] = '\0';
        fclose(f);
        if (strlen(g_secret) == 64) {
            fprintf(stdout, "[secret] Loaded visitor salt\n");
            return 0;
        }
    }

    FILE *rng = fopen("/dev/urandom", "rb");
    if (!rng) {
        fprintf(stderr, "[secret] Cannot open /dev/urandom\n");
        return -1;
    }
    unsigned char bytes[32];
    size_t n = fread(bytes, 1, 32, rng);
    fclose(rng);
    if (n != 32) return -1;

    for (int i = 0; i < 32; i++)
        snprintf(g_secret + i * 2, 3, "%02x", bytes[i]);
    g_secret[64] = '\0';

    f = fopen("data/secret.key", "w");
    if (!f) {
        fprintf(stderr, "[secret] Cannot write data/secret.key\n");
        return -1;
    }
    fprintf(f, "%s\n", g_secret);
    fclose(f);
    fprintf(stdout, "[secret] Generated new visitor salt -> data/secret.key\n");
    return 0;
}
