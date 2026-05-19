#ifndef EVENT_H
#define EVENT_H

#include "../router.h"

void handle_event(int client_fd, const HttpRequest *req);

#endif /* EVENT_H */
