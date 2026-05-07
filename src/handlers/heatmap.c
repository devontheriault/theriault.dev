#include "heatmap.h"
#include "../storage/db.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void parse_query_param(const char *path, const char *key,
                               char *out, size_t out_size) {
    out[0] = '\0';
    const char *q = strchr(path, '?');
    if (!q) return;
    q++;

    size_t key_len = strlen(key);
    const char *p = q;
    while (*p) {
        if (strncmp(p, key, key_len) == 0 && p[key_len] == '=') {
            p += key_len + 1;
            size_t i = 0;
            while (*p && *p != '&' && i < out_size - 1)
                out[i++] = *p++;
            out[i] = '\0';
            return;
        }
        while (*p && *p != '&') p++;
        if (*p == '&') p++;
    }
}

void handle_heatmap(int client_fd, const HttpRequest *req) {
    char country_filter[64] = {0};
    parse_query_param(req->path, "country", country_filter, sizeof(country_filter));

    int counts[7][24];
    int max_count = db_get_heatmap_data(counts, country_filter);

    char body[4096];
    int pos = 0;

    pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
        "{\"max\":%d,\"data\":[", max_count);

    for (int d = 0; d < 7; d++) {
        pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
            "%s[", d > 0 ? "," : "");
        for (int h = 0; h < 24; h++) {
            pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
                "%s%d", h > 0 ? "," : "", counts[d][h]);
        }
        pos += snprintf(body + pos, sizeof(body) - (size_t)pos, "]");
    }

    pos += snprintf(body + pos, sizeof(body) - (size_t)pos, "]}");

    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n",
        strlen(body));

    write(client_fd, header, strlen(header));
    write(client_fd, body,   strlen(body));
}
