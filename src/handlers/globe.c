#include "globe.h"
#include "../storage/db.h"
#include "../utils/json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GLOBE_MAX_CITIES 200

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

void handle_globe(int client_fd, const HttpRequest *req) {
    char country_filter[64]     = {0};
    char city_filter[64]        = {0};
    char browser_filter[32]     = {0};
    char device_type_filter[32] = {0};
    char os_filter[32]          = {0};
    parse_query_param(req->path, "country",     country_filter,     sizeof(country_filter));
    parse_query_param(req->path, "city",        city_filter,        sizeof(city_filter));
    parse_query_param(req->path, "browser",     browser_filter,     sizeof(browser_filter));
    parse_query_param(req->path, "device_type", device_type_filter, sizeof(device_type_filter));
    parse_query_param(req->path, "os",          os_filter,          sizeof(os_filter));

    char dow_str[8]  = {0};
    char hour_str[8] = {0};
    parse_query_param(req->path, "dow",  dow_str,  sizeof(dow_str));
    parse_query_param(req->path, "hour", hour_str, sizeof(hour_str));
    int dow  = (dow_str[0]  != '\0') ? (int)strtol(dow_str,  NULL, 10) : -1;
    int hour = (hour_str[0] != '\0') ? (int)strtol(hour_str, NULL, 10) : -1;
    if (dow  < 0 || dow  > 6)  dow  = -1;
    if (hour < 0 || hour > 23) hour = -1;

    char   cities[GLOBE_MAX_CITIES][64];
    char   countries[GLOBE_MAX_CITIES][64];
    int    counts[GLOBE_MAX_CITIES];
    double lats[GLOBE_MAX_CITIES];
    double lngs[GLOBE_MAX_CITIES];

    int n = db_get_city_globe_data(cities, countries, counts, lats, lngs, GLOBE_MAX_CITIES,
        country_filter, city_filter, dow, hour, browser_filter, device_type_filter, os_filter);

    char body[32768];
    int  pos = 0;

    pos += snprintf(body + pos, sizeof(body) - (size_t)pos, "{\"cities\":[");

    for (int i = 0; i < n && pos < (int)sizeof(body) - 128; i++) {
        char escaped_city[128];
        char escaped_country[128];
        json_string_escape(cities[i],    escaped_city,    sizeof(escaped_city));
        json_string_escape(countries[i], escaped_country, sizeof(escaped_country));
        pos += snprintf(body + pos, sizeof(body) - (size_t)pos,
            "%s{\"city\":\"%s\",\"country\":\"%s\",\"count\":%d,\"lat\":%.6f,\"lng\":%.6f}",
            (i > 0) ? "," : "",
            escaped_city,
            escaped_country,
            counts[i],
            lats[i],
            lngs[i]);
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
