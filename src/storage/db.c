#include "db.h"
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
    sqlite3_exec(g_db, "ALTER TABLE visits ADD COLUMN lat REAL", NULL, NULL, NULL);
    sqlite3_exec(g_db, "ALTER TABLE visits ADD COLUMN lng REAL", NULL, NULL, NULL);
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

int db_insert_visit(const char *ip, const char *country, const char *city, const char *user_agent, double lat, double lng) {
    if (!g_db) return -1;

    const char *sql = "INSERT INTO visits (ip, country, city, user_agent, lat, lng) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[db] Prepare error: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }

    sqlite3_bind_text(stmt,   1, ip,         -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,   2, country,    -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,   3, city,       -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt,   4, user_agent, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 5, lat);
    sqlite3_bind_double(stmt, 6, lng);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "[db] Insert error: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    return 0;
}

int db_get_total_visits(void) {
    if (!g_db) return 0;

    const char *sql = "SELECT COUNT(*) FROM visits WHERE country NOT IN ('Unknown', 'Localhost', 'Local', '');";
    sqlite3_stmt *stmt = NULL;

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 0;

    int total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        total = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return total;
}

int db_get_unique_countries(void) {
    if (!g_db) return 0;

    const char *sql = "SELECT COUNT(DISTINCT country) FROM visits WHERE country NOT IN ('Unknown', 'Localhost', 'Local', '');";
    sqlite3_stmt *stmt = NULL;

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 0;

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    return count;
}

int db_get_top_countries(char names[][64], int counts[], int max_n) {
    if (!g_db || max_n <= 0) return 0;

    const char *sql =
        "SELECT country, COUNT(*) as cnt "
        "FROM visits "
        "WHERE country NOT IN ('Unknown', 'Localhost', 'Local', '') "
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

int db_get_city_globe_data(char cities[][64], char countries[][64], int counts[], double lats[], double lngs[], int max_n) {
    if (!g_db || max_n <= 0) return 0;

    /* Group by both city and country so same-named cities in different
       countries are treated as distinct points. Fallback coordinates also
       filter by country to avoid cross-country coordinate bleed. */
    const char *sql =
        "SELECT v.city, v.country, COUNT(*) as cnt,"
        "  COALESCE(AVG(v.lat),"
        "    (SELECT r.latitude  FROM ip_ranges r WHERE r.city = v.city AND r.country = v.country AND r.latitude  != 0 LIMIT 1),"
        "    (SELECT r.latitude  FROM ip_ranges r WHERE r.city = v.city AND r.latitude  != 0 LIMIT 1)) as clat,"
        "  COALESCE(AVG(v.lng),"
        "    (SELECT r.longitude FROM ip_ranges r WHERE r.city = v.city AND r.country = v.country AND r.longitude != 0 LIMIT 1),"
        "    (SELECT r.longitude FROM ip_ranges r WHERE r.city = v.city AND r.longitude != 0 LIMIT 1)) as clng "
        "FROM visits v "
        "WHERE v.city NOT IN ('Unknown', 'Localhost', '') "
        "GROUP BY v.city, v.country "
        "HAVING clat IS NOT NULL AND (clat != 0 OR clng != 0) "
        "ORDER BY cnt DESC "
        "LIMIT ?;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 0;

    sqlite3_bind_int(stmt, 1, max_n);

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
        "WHERE country NOT IN ('Unknown', 'Localhost', 'Local', '') "
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

int db_get_heatmap_data(int counts[7][24], const char *country_filter) {
    if (!g_db) return 0;

    memset(counts, 0, 7 * 24 * sizeof(int));

    int has_filter = country_filter && country_filter[0] != '\0';

    const char *sql_all =
        "SELECT CAST(strftime('%w', timestamp) AS INTEGER) as dow,"
        "       CAST(strftime('%H', timestamp) AS INTEGER) as hr,"
        "       COUNT(*) as cnt "
        "FROM visits "
        "WHERE country NOT IN ('Unknown', 'Localhost', 'Local', '') "
        "GROUP BY dow, hr;";

    const char *sql_filtered =
        "SELECT CAST(strftime('%w', timestamp) AS INTEGER) as dow,"
        "       CAST(strftime('%H', timestamp) AS INTEGER) as hr,"
        "       COUNT(*) as cnt "
        "FROM visits "
        "WHERE country = ? "
        "GROUP BY dow, hr;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(g_db, has_filter ? sql_filtered : sql_all, -1, &stmt, NULL) != SQLITE_OK)
        return 0;

    if (has_filter)
        sqlite3_bind_text(stmt, 1, country_filter, -1, SQLITE_STATIC);

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

int db_get_top_cities(char names[][64], int counts[], int max_n, const char *country_filter) {
    if (!g_db || max_n <= 0) return 0;

    sqlite3_stmt *stmt = NULL;
    int has_filter = country_filter && country_filter[0] != '\0';

    const char *sql_filtered =
        "SELECT city, COUNT(*) as cnt "
        "FROM visits "
        "WHERE country = ? AND city NOT IN ('Unknown', 'Localhost', 'Local', '') "
        "GROUP BY city "
        "ORDER BY cnt DESC "
        "LIMIT ?;";

    const char *sql_all =
        "SELECT city, COUNT(*) as cnt "
        "FROM visits "
        "WHERE city NOT IN ('Unknown', 'Localhost', 'Local', '') "
        "GROUP BY city "
        "ORDER BY cnt DESC "
        "LIMIT ?;";

    if (sqlite3_prepare_v2(g_db, has_filter ? sql_filtered : sql_all, -1, &stmt, NULL) != SQLITE_OK)
        return 0;

    if (has_filter) {
        sqlite3_bind_text(stmt, 1, country_filter, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, max_n);
    } else {
        sqlite3_bind_int(stmt, 1, max_n);
    }

    int n = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && n < max_n) {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        int cnt = sqlite3_column_int(stmt, 1);

        strncpy(names[n], name ? name : "", 63);
        names[n][63] = '\0';
        counts[n] = cnt;
        n++;
    }

    sqlite3_finalize(stmt);
    return n;
}
