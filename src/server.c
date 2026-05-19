#include "server.h"
#include "router.h"
#include "utils/net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG      32
#define RECV_BUF  8192

void server_run(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[server] socket");
        exit(EXIT_FAILURE);
    }

    /* Allow fast restart */
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("[server] bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("[server] listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "[server] Listening on http://0.0.0.0:%d\n", port);
    fflush(stdout);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd,
                               (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            perror("[server] accept");
            continue;
        }

        /* Get peer IP string */
        char peer_ip[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &client_addr.sin_addr, peer_ip, sizeof(peer_ip));

        /* Read request */
        char buf[RECV_BUF];
        ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            close(client_fd);
            continue;
        }
        buf[n] = '\0';

        /* Parse request line: METHOD PATH HTTP/x.x */
        HttpRequest req;
        memset(&req, 0, sizeof(req));
        strncpy(req.raw, buf, sizeof(req.raw) - 1);

        if (sscanf(buf, "%7s %255s", req.method, req.path) < 2) {
            close(client_fd);
            continue;
        }
        url_decode(req.path);

        /* Extract real client IP, User-Agent, and Referer */
        net_extract_ip(buf, peer_ip, req.client_ip);
        net_extract_user_agent(buf, req.user_agent);
        net_extract_referrer(buf, req.referrer);

        int keep = router_dispatch(client_fd, &req);
        if (!keep) close(client_fd);
    }

    close(server_fd);
}
