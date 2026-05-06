#include "router.h"
#include "handlers/visit.h"
#include "handlers/stats.h"
#include "handlers/health.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Serve a static file from the web/ directory without recording a visit. */
static void serve_static(int client_fd, const char *web_path) {
    /* Build filesystem path: "web" + web_path */
    char fs_path[512];
    snprintf(fs_path, sizeof(fs_path), "web%s", web_path);

    FILE *f = fopen(fs_path, "rb");
    if (!f) {
        const char *body = "Not Found";
        char header[256];
        snprintf(header, sizeof(header),
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n",
            strlen(body));
        write(client_fd, header, strlen(header));
        write(client_fd, body,   strlen(body));
        return;
    }

    /* Determine Content-Type by extension */
    const char *ct = "application/octet-stream";
    const char *dot = strrchr(web_path, '.');
    if (dot) {
        if (strcmp(dot, ".css")  == 0) ct = "text/css";
        else if (strcmp(dot, ".js")   == 0) ct = "application/javascript";
        else if (strcmp(dot, ".html") == 0) ct = "text/html; charset=utf-8";
        else if (strcmp(dot, ".png")  == 0) ct = "image/png";
        else if (strcmp(dot, ".svg")  == 0) ct = "image/svg+xml";
        else if (strcmp(dot, ".ico")  == 0) ct = "image/x-icon";
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    if (size <= 0) { fclose(f); return; }

    char *buf = malloc((size_t)size);
    if (!buf) { fclose(f); return; }

    size_t rn = fread(buf, 1, (size_t)size, f);
    fclose(f);

    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        ct, rn);

    write(client_fd, header, strlen(header));
    write(client_fd, buf, rn);
    free(buf);
}

void router_dispatch(int client_fd, HttpRequest *req) {
    fprintf(stdout, "[router] %s %s  from %s\n",
            req->method, req->path, req->client_ip);
    fflush(stdout);

    if (strcmp(req->method, "GET") == 0) {
        if (strncmp(req->path, "/stats", 6) == 0) {
            handle_stats(client_fd, req);
            return;
        }
        if (strncmp(req->path, "/health", 7) == 0) {
            handle_health(client_fd, req);
            return;
        }
        /* Serve static assets (css, js, images) without recording a visit */
        if (strncmp(req->path, "/css/", 5) == 0 ||
            strncmp(req->path, "/js/",  4) == 0 ||
            strncmp(req->path, "/img/", 5) == 0) {
            serve_static(client_fd, req->path);
            return;
        }
    }

    /* Default: log visit and serve frontend */
    handle_visit(client_fd, req);
}
