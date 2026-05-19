#include "visit.h"
#include "../services/geo.h"
#include "../services/logger.h"
#include "../utils/bot_filter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HTML_PATH "web/index.html"

static void send_404(int client_fd) {
    const char *body =
        "<!DOCTYPE html><html><body>"
        "<h1>404 Not Found</h1>"
        "</body></html>";
    char header[256];
    snprintf(header, sizeof(header),
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        strlen(body));
    write(client_fd, header, strlen(header));
    write(client_fd, body,   strlen(body));
}

void handle_visit(int client_fd, const HttpRequest *req) {
    /* Perform geo lookup */
    char   country[64] = "Unknown";
    char   city[64]    = "Unknown";
    double lat = 0.0, lng = 0.0;
    geo_lookup(req->client_ip, country, city, &lat, &lng);

    /* Record the visit */
    char entry_page[256] = {0};
    strncpy(entry_page, req->path, sizeof(entry_page) - 1);
    char *qs = strchr(entry_page, '?');
    if (qs) *qs = '\0';

    if (is_bot_request(req->user_agent, entry_page)) {
        fprintf(stdout, "[visit] bot/scanner suppressed  ip=%-16s  ua=%s  path=%s\n",
                req->client_ip, req->user_agent, entry_page);
        fflush(stdout);
    } else {
        logger_record(req->client_ip, country, city, req->user_agent, lat, lng, req->referrer, entry_page);
    }

    /* Read index.html from disk */
    FILE *f = fopen(HTML_PATH, "rb");
    if (!f) {
        fprintf(stderr, "[visit] Cannot open %s\n", HTML_PATH);
        send_404(client_fd);
        return;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    if (size <= 0) {
        fclose(f);
        send_404(client_fd);
        return;
    }

    char *html = malloc((size_t)size + 1);
    if (!html) {
        fclose(f);
        send_404(client_fd);
        return;
    }

    size_t read_n = fread(html, 1, (size_t)size, f);
    fclose(f);
    html[read_n] = '\0';

    char header[256];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        read_n);

    write(client_fd, header, strlen(header));
    write(client_fd, html,   read_n);
    free(html);
}
