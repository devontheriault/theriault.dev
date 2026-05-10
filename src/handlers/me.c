#include "me.h"
#include "../services/geo.h"
#include "../utils/ua_parser.h"
#include "../utils/json.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void handle_me(int client_fd, const HttpRequest *req) {
    char country[64] = "Unknown";
    char city[64]    = "Unknown";
    double lat = 0.0, lng = 0.0;
    geo_lookup(req->client_ip, country, city, &lat, &lng);

    char browser[32]     = "Other";
    char device_type[32] = "Desktop";
    char os[32]          = "Unknown";
    ua_parse(req->user_agent, browser, device_type, os);

    char esc_country[128], esc_city[128];
    char esc_browser[64], esc_device[64], esc_os[64];
    json_string_escape(country,     esc_country, sizeof(esc_country));
    json_string_escape(city,        esc_city,    sizeof(esc_city));
    json_string_escape(browser,     esc_browser, sizeof(esc_browser));
    json_string_escape(device_type, esc_device,  sizeof(esc_device));
    json_string_escape(os,          esc_os,      sizeof(esc_os));

    char body[1024];
    int body_len = snprintf(body, sizeof(body),
        "{"
        "\"ip\":\"%s\","
        "\"country\":\"%s\","
        "\"city\":\"%s\","
        "\"lat\":%.4f,"
        "\"lng\":%.4f,"
        "\"browser\":\"%s\","
        "\"device_type\":\"%s\","
        "\"os\":\"%s\""
        "}",
        req->client_ip, esc_country, esc_city, lat, lng,
        esc_browser, esc_device, esc_os);

    char header[256];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n",
        body_len);

    write(client_fd, header, strlen(header));
    write(client_fd, body,   (size_t)body_len);
}
