#include "logger.h"
#include "../storage/db.h"
#include "../handlers/sse.h"
#include <stdio.h>
#include <time.h>

long long logger_record(const char *visitor_id, const char *country, const char *city, const char *user_agent, double lat, double lng, const char *referrer, const char *entry_page) {
    time_t now = time(NULL);
    char tbuf[32];
    struct tm *tm_info = localtime(&now);
    strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(stdout, "[visit] %s  vid=%.12s  country=%-20s  city=%-20s  ref=%-20s  page=%-20s\n",
            tbuf, visitor_id ? visitor_id : "?", country, city,
            referrer ? referrer : "Direct", entry_page ? entry_page : "/");
    fflush(stdout);

    long long row_id = db_insert_visit(visitor_id, country, city, user_agent, lat, lng, referrer, entry_page);
    if (row_id >= 0) sse_broadcast();
    return row_id;
}
