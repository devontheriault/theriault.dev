#include "duration.h"
#include "../storage/db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void handle_duration(int client_fd, const HttpRequest *req) {
    /* Find body after the blank line separating headers from body */
    const char *body = strstr(req->raw, "\r\n\r\n");
    if (!body) goto done;
    body += 4;

    /* Parse seconds=N */
    const char *p = strstr(body, "seconds=");
    if (!p) goto done;
    p += 8;

    int seconds = (int)strtol(p, NULL, 10);
    if (seconds > 0 && seconds < 86400) {
        db_update_visit_duration(req->client_ip, req->user_agent, seconds);
        fprintf(stdout, "[duration] ip=%-16s  seconds=%d\n", req->client_ip, seconds);
        fflush(stdout);
    }

done:;
    const char *resp =
        "HTTP/1.1 204 No Content\r\n"
        "Connection: close\r\n"
        "\r\n";
    write(client_fd, resp, strlen(resp));
}
