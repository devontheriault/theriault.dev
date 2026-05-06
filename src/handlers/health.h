#ifndef HEALTH_H
#define HEALTH_H

#include "../router.h"

void handle_health(int client_fd, const HttpRequest *req);

#endif /* HEALTH_H */
