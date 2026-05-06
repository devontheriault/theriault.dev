#ifndef STATS_H
#define STATS_H

#include "../router.h"

void handle_stats(int client_fd, const HttpRequest *req);

#endif /* STATS_H */
