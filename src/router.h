#ifndef ROUTER_H
#define ROUTER_H

typedef struct {
    char method[8];
    char path[256];
    char client_ip[64];
    char user_agent[256];
    char raw[4096];   /* full raw request for header extraction */
} HttpRequest;

void router_dispatch(int client_fd, HttpRequest *req);

#endif /* ROUTER_H */
