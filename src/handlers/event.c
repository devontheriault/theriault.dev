#include "event.h"
#include "../services/logger.h"
#include "../services/geo.h"
#include "../storage/db.h"
#include "../utils/bot_filter.h"
#include "../utils/hash.h"
#include "../utils/net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void parse_field(const char *body, const char *key, char *out, size_t out_size) {
    out[0] = '\0';
    size_t klen = strlen(key);
    const char *p = body;
    while (*p) {
        if (strncmp(p, key, klen) == 0 && p[klen] == '=') {
            p += klen + 1;
            size_t i = 0;
            while (*p && *p != '&' && i < out_size - 1)
                out[i++] = *p++;
            out[i] = '\0';
            url_decode(out);
            return;
        }
        while (*p && *p != '&') p++;
        if (*p == '&') p++;
    }
}

static void process_referrer(const char *url, char *out, size_t out_size) {
    if (!url || !url[0]) {
        strncpy(out, "Direct", out_size - 1);
        out[out_size - 1] = '\0';
        return;
    }
    const char *h = url;
    if (strncmp(h, "https://", 8) == 0) h += 8;
    else if (strncmp(h, "http://", 7) == 0) h += 7;
    if (strncmp(h, "www.", 4) == 0) h += 4;
    size_t i = 0;
    while (h[i] && h[i] != '/' && h[i] != '?' && i < out_size - 1) i++;
    strncpy(out, h, i);
    out[i] = '\0';
    if (!out[0]) {
        strncpy(out, "Direct", out_size - 1);
        out[out_size - 1] = '\0';
    }
}

static void send_204(int client_fd) {
    const char *resp = "HTTP/1.1 204 No Content\r\nConnection: close\r\n\r\n";
    write(client_fd, resp, strlen(resp));
}

void handle_event(int client_fd, const HttpRequest *req) {
    const char *body = strstr(req->raw, "\r\n\r\n");
    if (!body) { send_204(client_fd); return; }
    body += 4;

    char type[16] = {0};
    parse_field(body, "type", type, sizeof(type));

    if (strcmp(type, "pageview") == 0) {
        if (is_bot_request(req->user_agent, req->path)) {
            send_204(client_fd);
            return;
        }

        char path[256] = {0};
        char raw_ref[512] = {0};
        parse_field(body, "path",     path,    sizeof(path));
        parse_field(body, "referrer", raw_ref, sizeof(raw_ref));

        if (!path[0]) strncpy(path, "/", sizeof(path) - 1);

        char referrer[128] = {0};
        process_referrer(raw_ref, referrer, sizeof(referrer));

        char visitor_id[65];
        hash_visitor_id(req->client_ip, req->user_agent, visitor_id);

        char country[64] = "Unknown";
        char city[64]    = "Unknown";
        double lat = 0.0, lng = 0.0;
        geo_lookup(req->client_ip, country, city, &lat, &lng);

        long long row_id = logger_record(visitor_id, country, city,
                                         req->user_agent, lat, lng,
                                         referrer, path);

        char resp_body[64];
        snprintf(resp_body, sizeof(resp_body), "{\"visit_id\":%lld}", row_id);
        char header[256];
        snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n"
            "\r\n",
            strlen(resp_body));
        write(client_fd, header, strlen(header));
        write(client_fd, resp_body, strlen(resp_body));

    } else if (strcmp(type, "duration") == 0) {
        char vid_str[32] = {0};
        char sec_str[16] = {0};
        char exit_page[256] = {0};
        parse_field(body, "visit_id",  vid_str,   sizeof(vid_str));
        parse_field(body, "seconds",   sec_str,   sizeof(sec_str));
        parse_field(body, "exit_page", exit_page, sizeof(exit_page));

        long long visit_id = strtoll(vid_str, NULL, 10);
        int seconds        = (int)strtol(sec_str, NULL, 10);

        if (visit_id > 0 && seconds > 0 && seconds < 86400) {
            db_update_visit_duration_by_id(visit_id, seconds,
                                           exit_page[0] ? exit_page : NULL);
            fprintf(stdout, "[event] duration  visit_id=%lld  seconds=%d\n",
                    visit_id, seconds);
            fflush(stdout);
        }
        send_204(client_fd);

    } else {
        send_204(client_fd);
    }
}
