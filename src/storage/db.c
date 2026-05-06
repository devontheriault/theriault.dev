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
        "  timestamp  DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    char *errmsg = NULL;
    rc = sqlite3_exec(g_db, schema, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[db] Schema error: %s\n", errmsg);
        sqlite3_free(errmsg);
        return -1;
    }

    fprintf(stdout, "[db] Database initialised at data/visits.db\n");
    return 0;
}

void db_close(void) {
    if (g_db) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}

int db_insert_visit(const char *ip, const char *country, const char *city, const char *user_agent) {
    if (!g_db) return -1;

    const char *sql = "INSERT INTO visits (ip, country, city, user_agent) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[db] Prepare error: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, ip,         -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, country,    -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, city,       -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, user_agent, -1, SQLITE_STATIC);

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

    const char *sql = "SELECT COUNT(DISTINCT ip || user_agent || timestamp) FROM visits;";
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

    const char *sql = "SELECT COUNT(DISTINCT country) FROM visits;";
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

        if (name) {
            strncpy(names[n], name, 63);
            names[n][63] = '\0';
        } else {
            strncpy(names[n], "Unknown", 63);
        }
        counts[n] = cnt;
        n++;
    }

    sqlite3_finalize(stmt);
    return n;
}

int db_get_top_cities(char names[][64], int counts[], int max_n, const char *country_filter) {
    if (!g_db || max_n <= 0) return 0;

    sqlite3_stmt *stmt = NULL;
    int has_filter = country_filter && country_filter[0] != '\0';

    const char *sql_filtered =
        "SELECT city, COUNT(*) as cnt "
        "FROM visits "
        "WHERE country = ? "
        "GROUP BY city "
        "ORDER BY cnt DESC "
        "LIMIT ?;";

    const char *sql_all =
        "SELECT city, COUNT(*) as cnt "
        "FROM visits "
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

        if (name) {
            strncpy(names[n], name, 63);
            names[n][63] = '\0';
        } else {
            strncpy(names[n], "Unknown", 63);
        }
        counts[n] = cnt;
        n++;
    }

    sqlite3_finalize(stmt);
    return n;
}
