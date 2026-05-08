#include "stats.h"
#include "../services/analytics.h"
#include "../utils/json.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Extract value of a query parameter from a path like /stats?country=Canada&foo=bar */
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
        /* Advance to next parameter */
        while (*p && *p != '&') p++;
        if (*p == '&') p++;
    }
}

void handle_stats(int client_fd, const HttpRequest *req) {
    char country_filter[64] = {0};
    char city_filter[64]    = {0};
    parse_query_param(req->path, "country", country_filter, sizeof(country_filter));
    parse_query_param(req->path, "city",    city_filter,    sizeof(city_filter));

    StatsResult stats = analytics_get_stats(country_filter, city_filter);

    /* Build the JSON body */
    char body[32768];
    int  pos = 0;

    pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
        "{"
        "\"total_visits\":%d,"
        "\"unique_countries\":%d,"
        "\"top_countries\":[",
        stats.total_visits,
        stats.unique_countries);

    for (int i = 0; i < stats.top_len && pos < (int)sizeof(body) - 64; i++) {
        char escaped[128];
        json_string_escape(stats.top_countries[i], escaped, sizeof(escaped));
        pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
            "%s{\"country\":\"%s\",\"count\":%d}",
            (i > 0) ? "," : "",
            escaped,
            stats.top_counts[i]);
    }

    pos += snprintf(body + pos, sizeof(body) - (size_t)pos, "],\"top_cities\":[");

    for (int i = 0; i < stats.city_len && pos < (int)sizeof(body) - 64; i++) {
        char escaped[128];
        json_string_escape(stats.top_cities[i], escaped, sizeof(escaped));
        pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
            "%s{\"city\":\"%s\",\"count\":%d}",
            (i > 0) ? "," : "",
            escaped,
            stats.city_counts[i]);
    }

    pos += snprintf(body + pos, sizeof(body) - (size_t)pos, "],\"browsers\":[");
    for (int i = 0; i < stats.browser_len && pos < (int)sizeof(body) - 64; i++) {
        char escaped[64];
        json_string_escape(stats.top_browsers[i], escaped, sizeof(escaped));
        pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
            "%s{\"browser\":\"%s\",\"count\":%d}",
            (i > 0) ? "," : "", escaped, stats.browser_counts[i]);
    }

    pos += snprintf(body + pos, sizeof(body) - (size_t)pos, "],\"device_types\":[");
    for (int i = 0; i < stats.device_type_len && pos < (int)sizeof(body) - 64; i++) {
        char escaped[64];
        json_string_escape(stats.device_types[i], escaped, sizeof(escaped));
        pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
            "%s{\"device_type\":\"%s\",\"count\":%d}",
            (i > 0) ? "," : "", escaped, stats.device_type_counts[i]);
    }

    pos += snprintf(body + pos, sizeof(body) - (size_t)pos, "],\"os\":[");
    for (int i = 0; i < stats.os_len && pos < (int)sizeof(body) - 64; i++) {
        char escaped[64];
        json_string_escape(stats.top_os[i], escaped, sizeof(escaped));
        pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
            "%s{\"os\":\"%s\",\"count\":%d}",
            (i > 0) ? "," : "", escaped, stats.os_counts[i]);
    }

    char escaped_country[128];
    json_string_escape(stats.active_country, escaped_country, sizeof(escaped_country));
    pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
        "],\"active_country\":\"%s\"}", escaped_country);

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
