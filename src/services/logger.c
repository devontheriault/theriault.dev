#include "logger.h"
#include "../storage/db.h"
#include "../handlers/sse.h"
#include <stdio.h>
#include <time.h>

int logger_record(const char *ip, const char *country, const char *city, const char *user_agent, double lat, double lng) {
    time_t now = time(NULL);
    char tbuf[32];
    struct tm *tm_info = localtime(&now);
    strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(stdout, "[visit] %s  ip=%-16s  country=%-20s  city=%-20s  ua=%s\n",
            tbuf, ip, country, city, user_agent ? user_agent : "");
    fflush(stdout);

    int rc = db_insert_visit(ip, country, city, user_agent, lat, lng);
    if (rc == 0) sse_broadcast();
    return rc;
}
