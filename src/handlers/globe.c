#include "globe.h"
#include "../storage/db.h"
#include "../utils/json.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define GLOBE_MAX_CITIES 200

void handle_globe(int client_fd, const HttpRequest *req) {
    (void)req;

    char   cities[GLOBE_MAX_CITIES][64];
    char   countries[GLOBE_MAX_CITIES][64];
    int    counts[GLOBE_MAX_CITIES];
    double lats[GLOBE_MAX_CITIES];
    double lngs[GLOBE_MAX_CITIES];

    int n = db_get_city_globe_data(cities, countries, counts, lats, lngs, GLOBE_MAX_CITIES);

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
