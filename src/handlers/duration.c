#include "duration.h"
#include "../storage/db.h"
#include "../utils/net.h"
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

    /* Parse exit_page=<path> */
    char exit_page[256] = {0};
    const char *ep = strstr(body, "exit_page=");
    if (ep) {
        ep += 10;
        size_t i = 0;
        while (*ep && *ep != '&' && i < sizeof(exit_page) - 1)
            exit_page[i++] = *ep++;
        exit_page[i] = '\0';
        url_decode(exit_page);
    }

    if (seconds > 0 && seconds < 86400) {
        db_update_visit_duration(req->client_ip, req->user_agent, seconds,
                                 exit_page[0] ? exit_page : NULL);
        fprintf(stdout, "[duration] ip=%-16s  seconds=%d  exit_page=%s\n",
                req->client_ip, seconds, exit_page[0] ? exit_page : "/");
        fflush(stdout);
    }

done:;
    const char *resp =
        "HTTP/1.1 204 No Content\r\n"
        "Connection: close\r\n"
        "\r\n";
    write(client_fd, resp, strlen(resp));
}
