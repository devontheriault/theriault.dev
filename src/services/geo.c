#include "geo.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sqlite3.h>

#define GEO_DB_PATH "data/visits.db"

static sqlite3 *geo_db = NULL;

static uint32_t ip_to_int(const char *ip) {
    struct in_addr addr;
    if (inet_pton(AF_INET, ip, &addr) != 1) return 0;
    return ntohl(addr.s_addr);
}

static int geo_open_db(void) {
    if (geo_db) return 0;
    if (sqlite3_open_v2(GEO_DB_PATH, &geo_db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
        fprintf(stderr, "[geo] Cannot open %s: %s\n", GEO_DB_PATH, sqlite3_errmsg(geo_db));
        sqlite3_close(geo_db);
        geo_db = NULL;
        return -1;
    }
    return 0;
}

int geo_lookup(const char *ip, char *country_out, char *city_out) {
    strncpy(country_out, "Unknown", 63); country_out[63] = '\0';
    strncpy(city_out,    "Unknown", 63); city_out[63]    = '\0';

    if (!ip || *ip == '\0') {
        strncpy(country_out, "Local", 63);
        strncpy(city_out,    "Localhost", 63);
        return 0;
    }

    if (strcmp(ip, "127.0.0.1") == 0 ||
        strcmp(ip, "::1")       == 0 ||
        strncmp(ip, "127.", 4)  == 0) {
        strncpy(country_out, "Local", 63);
        strncpy(city_out,    "Localhost", 63);
        return 0;
    }

    uint32_t ip_int = ip_to_int(ip);
    if (ip_int == 0) return -1;

    if (geo_open_db() != 0) return -1;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT country, city, end_ip FROM ip_ranges "
        "WHERE start_ip <= ? ORDER BY start_ip DESC LIMIT 1";

    if (sqlite3_prepare_v2(geo_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "[geo] Prepare failed: %s\n", sqlite3_errmsg(geo_db));
        return -1;
    }

    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)ip_int);

    int ret = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        uint32_t end_ip = (uint32_t)sqlite3_column_int64(stmt, 2);
        if (end_ip >= ip_int) {
            const char *country = (const char *)sqlite3_column_text(stmt, 0);
            const char *city    = (const char *)sqlite3_column_text(stmt, 1);
            if (country && country[0]) { strncpy(country_out, country, 63); country_out[63] = '\0'; }
            if (city    && city[0])    { strncpy(city_out,    city,    63); city_out[63]    = '\0'; }
            ret = 0;
        }
    }

    sqlite3_finalize(stmt);
    return ret;
}
