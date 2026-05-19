#ifndef ROUTER_H
#define ROUTER_H

typedef struct {
    char method[8];
    char path[256];
    char client_ip[64];
    char user_agent[256];
    char referrer[128];
    char raw[4096];   /* full raw request for header extraction */
} HttpRequest;

int router_dispatch(int client_fd, HttpRequest *req);  /* returns 1 if fd kept open (SSE), 0 otherwise */

#endif /* ROUTER_H */
