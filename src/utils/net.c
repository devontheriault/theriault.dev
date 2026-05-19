#include "net.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

void url_decode(char *str) {
    char *r = str, *w = str;
    while (*r) {
        if (*r == '+') {
            *w++ = ' '; r++;
        } else if (*r == '%' && isxdigit((unsigned char)r[1])
                             && isxdigit((unsigned char)r[2])) {
            char hex[3] = { r[1], r[2], '\0' };
            *w++ = (char)strtol(hex, NULL, 16);
            r += 3;
        } else {
            *w++ = *r++;
        }
    }
    *w = '\0';
}

void net_extract_ip(const char *raw_request, const char *peer_addr, char *ip_out) {
    ip_out[0] = '\0';

    if (raw_request) {
        /* Case-insensitive search for X-Forwarded-For header */
        const char *needle = "x-forwarded-for:";
        const char *p = raw_request;

        /* Walk through each line */
        while (*p) {
            /* Try to match header name (case-insensitive) */
            size_t nl = strlen(needle);
            int match = 1;
            for (size_t i = 0; i < nl && p[i]; i++) {
                if (tolower((unsigned char)p[i]) != needle[i]) {
                    match = 0;
                    break;
                }
            }

            if (match) {
                /* Advance past the header name */
                p += nl;
                /* Skip optional whitespace */
                while (*p == ' ' || *p == '\t') p++;
                /* Copy until comma, \r, or \n */
                size_t i = 0;
                while (*p && *p != ',' && *p != '\r' && *p != '\n' && i < 63) {
                    ip_out[i++] = *p++;
                }
                ip_out[i] = '\0';
                /* Trim trailing whitespace */
                while (i > 0 && (ip_out[i-1] == ' ' || ip_out[i-1] == '\t')) {
                    ip_out[--i] = '\0';
                }
                if (i > 0) return;
            }

            /* Advance to next line */
            while (*p && *p != '\n') p++;
            if (*p == '\n') p++;
        }
    }

    /* Fallback to peer address */
    if (peer_addr && *peer_addr) {
        strncpy(ip_out, peer_addr, 63);
        ip_out[63] = '\0';
    } else {
        strncpy(ip_out, "0.0.0.0", 63);
    }
}

void net_extract_user_agent(const char *raw_request, char *ua_out) {
    ua_out[0] = '\0';
    if (!raw_request) return;

    const char *needle = "user-agent:";
    const char *p = raw_request;

    while (*p) {
        size_t nl = strlen(needle);
        int match = 1;
        for (size_t i = 0; i < nl && p[i]; i++) {
            if (tolower((unsigned char)p[i]) != needle[i]) {
                match = 0;
                break;
            }
        }

        if (match) {
            p += nl;
            while (*p == ' ' || *p == '\t') p++;
            size_t i = 0;
            while (*p && *p != '\r' && *p != '\n' && i < 255) {
                ua_out[i++] = *p++;
            }
            ua_out[i] = '\0';
            while (i > 0 && (ua_out[i-1] == ' ' || ua_out[i-1] == '\t')) {
                ua_out[--i] = '\0';
            }
            return;
        }

        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;
    }
}

void net_extract_referrer(const char *raw_request, char *out) {
    char raw[256] = {0};

    if (raw_request) {
        const char *needle = "referer:";
        const char *p = raw_request;
        while (*p) {
            size_t nl = strlen(needle);
            int match = 1;
            for (size_t i = 0; i < nl && p[i]; i++) {
                if (tolower((unsigned char)p[i]) != needle[i]) { match = 0; break; }
            }
            if (match) {
                p += nl;
                while (*p == ' ' || *p == '\t') p++;
                size_t i = 0;
                while (*p && *p != '\r' && *p != '\n' && i < sizeof(raw) - 1)
                    raw[i++] = *p++;
                raw[i] = '\0';
                break;
            }
            while (*p && *p != '\n') p++;
            if (*p == '\n') p++;
        }
    }

    if (raw[0] == '\0') {
        strncpy(out, "Direct", 127);
        out[127] = '\0';
        return;
    }

    /* Strip protocol */
    const char *host = raw;
    if (strncmp(host, "https://", 8) == 0) host += 8;
    else if (strncmp(host, "http://", 7) == 0) host += 7;

    /* Strip leading www. */
    if (strncmp(host, "www.", 4) == 0) host += 4;

    /* Take up to first '/', '?', or end */
    size_t i = 0;
    while (host[i] && host[i] != '/' && host[i] != '?' && i < 127)
        i++;

    strncpy(out, host, i);
    out[i] = '\0';

    if (out[0] == '\0') {
        strncpy(out, "Direct", 127);
        out[127] = '\0';
    }
}
