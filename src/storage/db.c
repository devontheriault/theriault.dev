#include "db.h"
#include "../utils/ua_parser.h"
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

static sqlite3 *g_db = NULL;

int db_init(void) {
    int rc = sqlite3_open("data/visits.db", &g_db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[db] Cannot open database: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }

    const char *schema =
        "CREATE TABLE IF NOT EXISTS visits ("
        "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  ip         TEXT,"
        "  country    TEXT,"
        "  city       TEXT,"
        "  user_agent TEXT,"
        "  timestamp  DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "  lat        REAL,"
        "  lng        REAL"
        ");";

    char *errmsg = NULL;
    rc = sqlite3_exec(g_db, schema, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[db] Schema error: %s\n", errmsg);
        sqlite3_free(errmsg);
        return -1;
    }

    /* Migrations — silently ignored if columns already exist */
    sqlite3_exec(g_db, "ALTER TABLE visits ADD COLUMN lat REAL",          NULL, NULL, NULL);
    sqlite3_exec(g_db, "ALTER TABLE visits ADD COLUMN lng REAL",          NULL, NULL, NULL);
    sqlite3_exec(g_db, "ALTER TABLE visits ADD COLUMN browser TEXT",      NULL, NULL, NULL);
    sqlite3_exec(g_db, "ALTER TABLE visits ADD COLUMN device_type TEXT",  NULL, NULL, NULL);
    sqlite3_exec(g_db, "ALTER TABLE visits ADD COLUMN os TEXT",           NULL, NULL, NULL);
    sqlite3_exec(g_db, "ALTER TABLE visits ADD COLUMN time_on_page INTEGER", NULL, NULL, NULL);
    sqlite3_exec(g_db, "ALTER TABLE visits ADD COLUMN referrer TEXT",        NULL, NULL, NULL);
    sqlite3_exec(g_db, "ALTER TABLE visits ADD COLUMN entry_page TEXT",      NULL, NULL, NULL);
    sqlite3_exec(g_db, "ALTER TABLE visits ADD COLUMN exit_page TEXT",       NULL, NULL, NULL);
    sqlite3_exec(g_db, "ALTER TABLE ip_ranges ADD COLUMN latitude REAL DEFAULT 0", NULL, NULL, NULL);
    sqlite3_exec(g_db, "ALTER TABLE ip_ranges ADD COLUMN longitude REAL DEFAULT 0", NULL, NULL, NULL);
    /* Index so city-name coord fallback queries are fast */
    sqlite3_exec(g_db,
        "CREATE INDEX IF NOT EXISTS idx_ip_ranges_city ON ip_ranges(city)",
        NULL, NULL, NULL);

    fprintf(stdout, "[db] Database initialised at data/visits.db\n");
    return 0;
}

void db_close(void) {
    if (g_db) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}

int db_insert_visit(const char *ip, const char *country, const char *city, const char *user_agent, double lat, double lng, const char *referrer, const char *entry_page) {
    if (!g_db) return -1;

    char browser[32], device_type[32], os[32];
    ua_parse(user_agent, browser, device_type, os);

    const char *sql =
        "INSERT INTO visits (ip, country, city, user_agent, lat, lng, browser, device_type, os, referrer, entry_page)"
        " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[db] Prepare error: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }

    sqlite3_bind_text(stmt,   1, ip,          -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,   2, country,     -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,   3, city,        -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,   4, user_agent,  -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 5, lat);
    sqlite3_bind_double(stmt, 6, lng);
    sqlite3_bind_text(stmt,   7, browser,     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,   8, device_type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,   9, os,          -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,  10, referrer && referrer[0] ? referrer : "Direct", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,  11, entry_page && entry_page[0] ? entry_page : "/", -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "[db] Insert error: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    return 0;
}

int db_get_top_referrers(char names[][128], int counts[], int max_n) {
    if (!g_db || max_n <= 0) return 0;

    const char *sql =
        "SELECT COALESCE(referrer,'Direct') AS ref, COUNT(*) AS cnt"
        " FROM visits"
        " WHERE 1=1"
        " GROUP BY ref ORDER BY cnt DESC LIMIT ?;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, max_n);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_n) {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        strncpy(names[n], name ? name : "Direct", 127);
        names[n][127] = '\0';
        counts[n] = sqlite3_column_int(stmt, 1);
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

int db_update_visit_duration(const char *ip, const char *user_agent, int seconds, const char *exit_page) {
    if (!g_db) return -1;

    const char *sql =
        "UPDATE visits SET time_on_page = ?, exit_page = ?"
        " WHERE id = ("
        "   SELECT id FROM visits"
        "   WHERE ip = ? AND user_agent = ?"
        "   ORDER BY timestamp DESC LIMIT 1"
        " ) AND time_on_page IS NULL;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "[db] Prepare error: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }

    sqlite3_bind_int (stmt, 1, seconds);
    sqlite3_bind_text(stmt, 2, exit_page && exit_page[0] ? exit_page : "/", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, ip,         -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, user_agent, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? 0 : -1;
}

double db_get_avg_time_on_page(void) {
    if (!g_db) return 0.0;

    const char *sql =
        "SELECT AVG(time_on_page) FROM visits"
        " WHERE time_on_page IS NOT NULL;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 0.0;

    double avg = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL)
        avg = sqlite3_column_double(stmt, 0);

    sqlite3_finalize(stmt);
    return avg;
}

/* Appends filter conditions to a SQL buffer. unknown_guard=1 adds the country
   exclusion list when no geo filter is active. Returns updated pos. */
static int sql_append_filters(char *buf, size_t sz, int pos,
    const char *country, const char *city, int dow, int hour,
    const char *browser, const char *device_type, const char *os,
    int unknown_guard)
{
    if (unknown_guard && !(country && country[0]) && !(city && city[0]))
        pos += snprintf(buf + pos, sz - (size_t)pos,
            " AND country NOT IN ('Unknown','Localhost','Local','','-')");
    if (country     && country[0])     pos += snprintf(buf + pos, sz - (size_t)pos, " AND country = ?");
    if (city        && city[0])        pos += snprintf(buf + pos, sz - (size_t)pos, " AND city = ?");
    if (dow >= 0 && dow < 7)
        pos += snprintf(buf + pos, sz - (size_t)pos,
            " AND CAST(strftime('%%w', timestamp) AS INTEGER) = ?");
    if (hour >= 0 && hour < 24)
        pos += snprintf(buf + pos, sz - (size_t)pos,
            " AND CAST(strftime('%%H', timestamp) AS INTEGER) = ?");
    if (browser     && browser[0])     pos += snprintf(buf + pos, sz - (size_t)pos, " AND browser = ?");
    if (device_type && device_type[0]) pos += snprintf(buf + pos, sz - (size_t)pos, " AND device_type = ?");
    if (os          && os[0])          pos += snprintf(buf + pos, sz - (size_t)pos, " AND os = ?");
    return pos;
}

/* Binds filter values in order: country, city, dow, hour, browser, device_type, os.
   Returns the next bind index. */
static int sql_bind_filters(sqlite3_stmt *stmt, int b,
    const char *country, const char *city, int dow, int hour,
    const char *browser, const char *device_type, const char *os)
{
    if (country     && country[0])     sqlite3_bind_text(stmt, b++, country,     -1, SQLITE_STATIC);
    if (city        && city[0])        sqlite3_bind_text(stmt, b++, city,        -1, SQLITE_STATIC);
    if (dow  >= 0 && dow  < 7)         sqlite3_bind_int (stmt, b++, dow);
    if (hour >= 0 && hour < 24)        sqlite3_bind_int (stmt, b++, hour);
    if (browser     && browser[0])     sqlite3_bind_text(stmt, b++, browser,     -1, SQLITE_STATIC);
    if (device_type && device_type[0]) sqlite3_bind_text(stmt, b++, device_type, -1, SQLITE_STATIC);
    if (os          && os[0])          sqlite3_bind_text(stmt, b++, os,          -1, SQLITE_STATIC);
    return b;
}

static int scalar_query_filtered(const char *select_expr,
    const char *country, const char *city, int dow, int hour,
    const char *browser, const char *device_type, const char *os)
{
    char sql[768];
    int pos = snprintf(sql, sizeof(sql),
        "SELECT %s FROM visits WHERE 1=1", select_expr);
    pos = sql_append_filters(sql, sizeof(sql), pos, country, city, dow, hour, browser, device_type, os, 1);

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sql_bind_filters(stmt, 1, country, city, dow, hour, browser, device_type, os);

    int result = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) result = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return result;
}

int db_get_total_visits(const char *country, const char *city, int dow, int hour,
    const char *browser, const char *device_type, const char *os) {
    if (!g_db) return 0;
    return scalar_query_filtered("COUNT(*)", country, city, dow, hour, browser, device_type, os);
}

int db_get_unique_visitors(const char *country, const char *city, int dow, int hour,
    const char *browser, const char *device_type, const char *os) {
    if (!g_db) return 0;
    return scalar_query_filtered(
        "COUNT(DISTINCT ip||'|'||user_agent||'|'||date(timestamp))",
        country, city, dow, hour, browser, device_type, os);
}

int db_get_unique_countries(const char *country, const char *city, int dow, int hour,
    const char *browser, const char *device_type, const char *os) {
    if (!g_db) return 0;
    return scalar_query_filtered("COUNT(DISTINCT country)", country, city, dow, hour, browser, device_type, os);
}

int db_get_top_countries(char names[][64], int counts[], int max_n, int dow, int hour,
    const char *browser, const char *device_type, const char *os) {
    if (!g_db || max_n <= 0) return 0;

    char sql[768];
    int pos = snprintf(sql, sizeof(sql),
        "SELECT country, COUNT(*) AS cnt FROM visits WHERE 1=1");
    pos = sql_append_filters(sql, sizeof(sql), pos, NULL, NULL, dow, hour, browser, device_type, os, 1);
    snprintf(sql + pos, sizeof(sql) - (size_t)pos,
        " GROUP BY country ORDER BY cnt DESC LIMIT ?");

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    int b = sql_bind_filters(stmt, 1, NULL, NULL, dow, hour, browser, device_type, os);
    sqlite3_bind_int(stmt, b, max_n);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_n) {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        strncpy(names[n], name ? name : "", 63);
        names[n][63] = '\0';
        counts[n] = sqlite3_column_int(stmt, 1);
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

int db_get_city_globe_data(char cities[][64], char countries[][64], int counts[], double lats[], double lngs[], int max_n,
    const char *country_filter, const char *city_filter, int dow, int hour,
    const char *browser_filter, const char *device_type_filter, const char *os_filter) {
    if (!g_db || max_n <= 0) return 0;

    char sql[1536];
    int pos = snprintf(sql, sizeof(sql),
        "SELECT v.city, v.country, COUNT(*) as cnt,"
        "  COALESCE(NULLIF(AVG(v.lat), 0),"
        "    (SELECT r.latitude  FROM ip_ranges r WHERE r.city = v.city AND r.country = v.country AND r.latitude  != 0 LIMIT 1),"
        "    (SELECT r.latitude  FROM ip_ranges r WHERE r.city = v.city AND r.latitude  != 0 LIMIT 1)) as clat,"
        "  COALESCE(NULLIF(AVG(v.lng), 0),"
        "    (SELECT r.longitude FROM ip_ranges r WHERE r.city = v.city AND r.country = v.country AND r.longitude != 0 LIMIT 1),"
        "    (SELECT r.longitude FROM ip_ranges r WHERE r.city = v.city AND r.longitude != 0 LIMIT 1)) as clng "
        "FROM visits v "
        "WHERE v.city NOT IN ('Unknown', 'Localhost', 'Local', '')");
    pos = sql_append_filters(sql, sizeof(sql), pos, country_filter, city_filter, dow, hour,
        browser_filter, device_type_filter, os_filter, 0);
    snprintf(sql + pos, sizeof(sql) - (size_t)pos,
        " GROUP BY v.city, v.country"
        " HAVING clat IS NOT NULL AND (clat != 0 OR clng != 0)"
        " ORDER BY cnt DESC"
        " LIMIT ?");

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 0;

    int b = sql_bind_filters(stmt, 1, country_filter, city_filter, dow, hour,
        browser_filter, device_type_filter, os_filter);
    sqlite3_bind_int(stmt, b, max_n);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_n) {
        const char *name    = (const char *)sqlite3_column_text(stmt, 0);
        const char *country = (const char *)sqlite3_column_text(stmt, 1);
        if (name) {
            strncpy(cities[n], name, 63);
            cities[n][63] = '\0';
        } else {
            strncpy(cities[n], "Unknown", 63);
        }
        if (country) {
            strncpy(countries[n], country, 63);
            countries[n][63] = '\0';
        } else {
            countries[n][0] = '\0';
        }
        counts[n] = sqlite3_column_int(stmt, 2);
        lats[n]   = sqlite3_column_double(stmt, 3);
        lngs[n]   = sqlite3_column_double(stmt, 4);
        n++;
    }

    sqlite3_finalize(stmt);
    return n;
}

int db_get_all_countries(char names[][64], int counts[], int max_n) {
    if (!g_db || max_n <= 0) return 0;

    const char *sql =
        "SELECT country, COUNT(*) as cnt "
        "FROM visits "
        "WHERE country NOT IN ('Unknown', 'Localhost', 'Local', '', '-') "
        "GROUP BY country "
        "ORDER BY cnt DESC "
        "LIMIT ?;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 0;

    sqlite3_bind_int(stmt, 1, max_n);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_n) {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        int cnt = sqlite3_column_int(stmt, 1);

        strncpy(names[n], name, 63);
        names[n][63] = '\0';
        counts[n] = cnt;
        n++;
    }

    sqlite3_finalize(stmt);
    return n;
}

int db_get_heatmap_data(int counts[7][24], const char *country_filter, const char *city_filter,
    const char *browser_filter, const char *device_type_filter, const char *os_filter) {
    if (!g_db) return 0;

    memset(counts, 0, 7 * 24 * sizeof(int));

    char sql[1024];
    int pos = snprintf(sql, sizeof(sql),
        "SELECT CAST(strftime('%%w', timestamp) AS INTEGER) as dow,"
        "       CAST(strftime('%%H', timestamp) AS INTEGER) as hr,"
        "       COUNT(*) as cnt "
        "FROM visits WHERE 1=1");
    pos = sql_append_filters(sql, sizeof(sql), pos,
        country_filter, city_filter, -1, -1,
        browser_filter, device_type_filter, os_filter, 1);
    snprintf(sql + pos, sizeof(sql) - (size_t)pos, " GROUP BY dow, hr");

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sql_bind_filters(stmt, 1, country_filter, city_filter, -1, -1,
        browser_filter, device_type_filter, os_filter);

    int max_count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int dow = sqlite3_column_int(stmt, 0);
        int hr  = sqlite3_column_int(stmt, 1);
        int cnt = sqlite3_column_int(stmt, 2);
        if (dow >= 0 && dow < 7 && hr >= 0 && hr < 24) {
            counts[dow][hr] = cnt;
            if (cnt > max_count) max_count = cnt;
        }
    }

    sqlite3_finalize(stmt);
    return max_count;
}

static int device_query(const char *col, const char *fallback,
    const char *country_filter, const char *city_filter,
    int dow, int hour,
    const char *browser_filter, const char *device_type_filter, const char *os_filter,
    char names[][32], int counts[], int max_n)
{
    char sql[900];
    int pos = snprintf(sql, sizeof(sql),
        "SELECT COALESCE(%s,'%s') AS v, COUNT(*) AS cnt FROM visits WHERE 1=1",
        col, fallback);
    pos = sql_append_filters(sql, sizeof(sql), pos, country_filter, city_filter, dow, hour,
        browser_filter, device_type_filter, os_filter, 1);
    snprintf(sql + pos, sizeof(sql) - (size_t)pos,
        " GROUP BY COALESCE(%s,'%s') ORDER BY cnt DESC LIMIT ?", col, fallback);

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    int b = sql_bind_filters(stmt, 1, country_filter, city_filter, dow, hour,
        browser_filter, device_type_filter, os_filter);
    sqlite3_bind_int(stmt, b, max_n);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_n) {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        strncpy(names[n], name ? name : fallback, 31);
        names[n][31] = '\0';
        counts[n] = sqlite3_column_int(stmt, 1);
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

int db_get_top_browsers(char names[][32], int counts[], int max_n,
    const char *country_filter, const char *city_filter, int dow, int hour,
    const char *device_type_filter, const char *os_filter)
{
    if (!g_db || max_n <= 0) return 0;
    return device_query("browser", "Other",
        country_filter, city_filter, dow, hour,
        NULL, device_type_filter, os_filter,
        names, counts, max_n);
}

int db_get_device_breakdown(char names[][32], int counts[], int max_n,
    const char *country_filter, const char *city_filter, int dow, int hour,
    const char *browser_filter, const char *os_filter)
{
    if (!g_db || max_n <= 0) return 0;
    return device_query("device_type", "Desktop",
        country_filter, city_filter, dow, hour,
        browser_filter, NULL, os_filter,
        names, counts, max_n);
}

int db_get_top_os(char names[][32], int counts[], int max_n,
    const char *country_filter, const char *city_filter, int dow, int hour,
    const char *browser_filter, const char *device_type_filter)
{
    if (!g_db || max_n <= 0) return 0;
    return device_query("os", "Unknown",
        country_filter, city_filter, dow, hour,
        browser_filter, device_type_filter, NULL,
        names, counts, max_n);
}

int db_get_top_cities(char names[][64], int counts[], int max_n,
    const char *country_filter, int dow, int hour,
    const char *browser_filter, const char *device_type_filter, const char *os_filter)
{
    if (!g_db || max_n <= 0) return 0;

    int has_country = country_filter && country_filter[0] != '\0';

    char sql[768];
    int pos = snprintf(sql, sizeof(sql),
        "SELECT city, COUNT(*) AS cnt FROM visits"
        " WHERE city NOT IN ('Unknown','Localhost','Local','','-')");
    if (has_country)
        pos += snprintf(sql + pos, sizeof(sql) - (size_t)pos, " AND country = ?");
    if (dow >= 0 && dow < 7)
        pos += snprintf(sql + pos, sizeof(sql) - (size_t)pos,
            " AND CAST(strftime('%%w', timestamp) AS INTEGER) = ?");
    if (hour >= 0 && hour < 24)
        pos += snprintf(sql + pos, sizeof(sql) - (size_t)pos,
            " AND CAST(strftime('%%H', timestamp) AS INTEGER) = ?");
    if (browser_filter && browser_filter[0])
        pos += snprintf(sql + pos, sizeof(sql) - (size_t)pos, " AND browser = ?");
    if (device_type_filter && device_type_filter[0])
        pos += snprintf(sql + pos, sizeof(sql) - (size_t)pos, " AND device_type = ?");
    if (os_filter && os_filter[0])
        pos += snprintf(sql + pos, sizeof(sql) - (size_t)pos, " AND os = ?");
    snprintf(sql + pos, sizeof(sql) - (size_t)pos,
        " GROUP BY city ORDER BY cnt DESC LIMIT ?");

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;

    int b = 1;
    if (has_country) sqlite3_bind_text(stmt, b++, country_filter, -1, SQLITE_STATIC);
    if (dow  >= 0 && dow  < 7)  sqlite3_bind_int(stmt, b++, dow);
    if (hour >= 0 && hour < 24) sqlite3_bind_int(stmt, b++, hour);
    if (browser_filter     && browser_filter[0])     sqlite3_bind_text(stmt, b++, browser_filter,     -1, SQLITE_STATIC);
    if (device_type_filter && device_type_filter[0]) sqlite3_bind_text(stmt, b++, device_type_filter, -1, SQLITE_STATIC);
    if (os_filter          && os_filter[0])          sqlite3_bind_text(stmt, b++, os_filter,          -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, b, max_n);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_n) {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        strncpy(names[n], name ? name : "", 63);
        names[n][63] = '\0';
        counts[n] = sqlite3_column_int(stmt, 1);
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

int db_get_top_entry_pages(char names[][128], int counts[], int max_n) {
    if (!g_db || max_n <= 0) return 0;

    const char *sql =
        "SELECT COALESCE(entry_page, '/') AS page, COUNT(*) AS cnt"
        " FROM visits"
        " GROUP BY page ORDER BY cnt DESC LIMIT ?;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, max_n);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_n) {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        strncpy(names[n], name ? name : "/", 127);
        names[n][127] = '\0';
        counts[n] = sqlite3_column_int(stmt, 1);
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

int db_get_views_per_page(char names[][128], int counts[], int max_n) {
    if (!g_db || max_n <= 0) return 0;

    const char *sql =
        "SELECT COALESCE(entry_page, '/') AS page, COUNT(*) AS cnt"
        " FROM visits"
        " GROUP BY page ORDER BY cnt DESC LIMIT ?;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, max_n);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_n) {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        strncpy(names[n], name ? name : "/", 127);
        names[n][127] = '\0';
        counts[n] = sqlite3_column_int(stmt, 1);
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}

int db_get_top_exit_pages(char names[][128], int counts[], int max_n) {
    if (!g_db || max_n <= 0) return 0;

    const char *sql =
        "SELECT exit_page AS page, COUNT(*) AS cnt"
        " FROM visits"
        " WHERE exit_page IS NOT NULL"
        " GROUP BY page ORDER BY cnt DESC LIMIT ?;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_int(stmt, 1, max_n);

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_n) {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        strncpy(names[n], name ? name : "/", 127);
        names[n][127] = '\0';
        counts[n] = sqlite3_column_int(stmt, 1);
        n++;
    }
    sqlite3_finalize(stmt);
    return n;
}
