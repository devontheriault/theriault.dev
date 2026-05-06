#include "health.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void handle_health(int client_fd, const HttpRequest *req) {
    (void)req;

    const char *body = "{\"status\":\"ok\"}";
    char header[256];
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
