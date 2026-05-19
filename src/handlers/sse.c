#include "sse.h"
#include "../services/analytics.h"
#include "../storage/db.h"
#include "../utils/json.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <stdint.h>

#define SSE_MAX_CLIENTS  32
#define GLOBE_MAX_CITIES 200
#define SSE_BUF_SIZE     98304  /* 6 (prefix) + ~96KB json + 2 (suffix) */

typedef struct {
    int fd;  /* -1 = slot is free */
} SseClient;

static SseClient       g_clients[SSE_MAX_CLIENTS];
static pthread_mutex_t g_clients_mutex = PTHREAD_MUTEX_INITIALIZER;

static void __attribute__((constructor)) sse_init(void) {
    for (int i = 0; i < SSE_MAX_CLIENTS; i++)
        g_clients[i].fd = -1;
}

static void sse_client_add(int fd) {
    pthread_mutex_lock(&g_clients_mutex);
    for (int i = 0; i < SSE_MAX_CLIENTS; i++) {
        if (g_clients[i].fd < 0) {
            g_clients[i].fd = fd;
            break;
        }
    }
    pthread_mutex_unlock(&g_clients_mutex);
}

static void sse_client_remove(int fd) {
    pthread_mutex_lock(&g_clients_mutex);
    for (int i = 0; i < SSE_MAX_CLIENTS; i++) {
        if (g_clients[i].fd == fd) {
            g_clients[i].fd = -1;
            break;
        }
    }
    pthread_mutex_unlock(&g_clients_mutex);
}

static void *sse_client_thread(void *arg) {
    int fd = (int)(intptr_t)arg;
    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        struct timeval tv = { .tv_sec = 15, .tv_usec = 0 };
        int r = select(fd + 1, &rfds, NULL, NULL, &tv);
        if (r < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (r > 0) break;  /* client closed / sent unexpected data */
        /* Timeout: send keepalive ping */
        pthread_mutex_lock(&g_clients_mutex);
        ssize_t w = write(fd, ": ping\n\n", 8);
        pthread_mutex_unlock(&g_clients_mutex);
        if (w <= 0) break;
    }
    sse_client_remove(fd);
    close(fd);
    return NULL;
}

/* Build combined stats+heatmap+globe JSON into buf starting at buf[0].
   Returns number of bytes written (not including NUL). */
static int sse_build_json(char *buf, size_t size) {
    int pos = 0;

    /* --- Stats (no country filter) --- */
    StatsResult stats = analytics_get_stats("", "", -1, -1, "", "", "");
    pos += snprintf(buf + pos, size - (size_t)pos,
        "{\"stats\":{"
        "\"total_visits\":%d,"
        "\"unique_visitors\":%d,"
        "\"unique_countries\":%d,"
        "\"top_countries\":[",
        stats.total_visits,
        stats.unique_visitors,
        stats.unique_countries);

    for (int i = 0; i < stats.top_len && pos < (int)size - 64; i++) {
        char esc[128];
        json_string_escape(stats.top_countries[i], esc, sizeof(esc));
        pos += snprintf(buf + pos, size - (size_t)pos,
            "%s{\"country\":\"%s\",\"count\":%d}",
            i > 0 ? "," : "", esc, stats.top_counts[i]);
    }
    pos += snprintf(buf + pos, size - (size_t)pos, "],\"top_cities\":[");
    for (int i = 0; i < stats.city_len && pos < (int)size - 64; i++) {
        char esc[128];
        json_string_escape(stats.top_cities[i], esc, sizeof(esc));
        pos += snprintf(buf + pos, size - (size_t)pos,
            "%s{\"city\":\"%s\",\"count\":%d}",
            i > 0 ? "," : "", esc, stats.city_counts[i]);
    }

    pos += snprintf(buf + pos, size - (size_t)pos, "],\"browsers\":[");
    for (int i = 0; i < stats.browser_len && pos < (int)size - 64; i++) {
        char esc[64];
        json_string_escape(stats.top_browsers[i], esc, sizeof(esc));
        pos += snprintf(buf + pos, size - (size_t)pos,
            "%s{\"browser\":\"%s\",\"count\":%d}",
            i > 0 ? "," : "", esc, stats.browser_counts[i]);
    }

    pos += snprintf(buf + pos, size - (size_t)pos, "],\"device_types\":[");
    for (int i = 0; i < stats.device_type_len && pos < (int)size - 64; i++) {
        char esc[64];
        json_string_escape(stats.device_types[i], esc, sizeof(esc));
        pos += snprintf(buf + pos, size - (size_t)pos,
            "%s{\"device_type\":\"%s\",\"count\":%d}",
            i > 0 ? "," : "", esc, stats.device_type_counts[i]);
    }

    pos += snprintf(buf + pos, size - (size_t)pos, "],\"os\":[");
    for (int i = 0; i < stats.os_len && pos < (int)size - 64; i++) {
        char esc[64];
        json_string_escape(stats.top_os[i], esc, sizeof(esc));
        pos += snprintf(buf + pos, size - (size_t)pos,
            "%s{\"os\":\"%s\",\"count\":%d}",
            i > 0 ? "," : "", esc, stats.os_counts[i]);
    }

    pos += snprintf(buf + pos, size - (size_t)pos, "],\"referrers\":[");
    for (int i = 0; i < stats.referrer_len && pos < (int)size - 64; i++) {
        char esc[256];
        json_string_escape(stats.top_referrers[i], esc, sizeof(esc));
        pos += snprintf(buf + pos, size - (size_t)pos,
            "%s{\"referrer\":\"%s\",\"count\":%d}",
            i > 0 ? "," : "", esc, stats.referrer_counts[i]);
    }

    pos += snprintf(buf + pos, size - (size_t)pos,
        "],\"active_country\":\"\",\"avg_time_on_page\":%.1f}",
        stats.avg_time_on_page);

    /* --- Heatmap (no country filter) --- */
    int hm[7][24];
    int hm_max = db_get_heatmap_data(hm, "", "", "", "", "");
    pos += snprintf(buf + pos, size - (size_t)pos,
        ",\"heatmap\":{\"max\":%d,\"data\":[", hm_max);
    for (int d = 0; d < 7; d++) {
        pos += snprintf(buf + pos, size - (size_t)pos,
            "%s[", d > 0 ? "," : "");
        for (int h = 0; h < 24; h++) {
            pos += snprintf(buf + pos, size - (size_t)pos,
                "%s%d", h > 0 ? "," : "", hm[d][h]);
        }
        pos += snprintf(buf + pos, size - (size_t)pos, "]");
    }
    pos += snprintf(buf + pos, size - (size_t)pos, "]}");

    /* --- Globe --- */
    char   cities[GLOBE_MAX_CITIES][64];
    char   gctries[GLOBE_MAX_CITIES][64];
    int    gcounts[GLOBE_MAX_CITIES];
    double glats[GLOBE_MAX_CITIES];
    double glngs[GLOBE_MAX_CITIES];
    int    n = db_get_city_globe_data(cities, gctries, gcounts, glats, glngs, GLOBE_MAX_CITIES,
        NULL, NULL, -1, -1, NULL, NULL, NULL);

    pos += snprintf(buf + pos, size - (size_t)pos, ",\"globe\":{\"cities\":[");
    for (int i = 0; i < n && pos < (int)size - 128; i++) {
        char esc_city[128], esc_ctry[128];
        json_string_escape(cities[i],  esc_city,  sizeof(esc_city));
        json_string_escape(gctries[i], esc_ctry, sizeof(esc_ctry));
        pos += snprintf(buf + pos, size - (size_t)pos,
            "%s{\"city\":\"%s\",\"country\":\"%s\",\"count\":%d,\"lat\":%.6f,\"lng\":%.6f}",
            i > 0 ? "," : "",
            esc_city, esc_ctry, gcounts[i], glats[i], glngs[i]);
    }
    pos += snprintf(buf + pos, size - (size_t)pos, "]}}");

    return pos;
}

/* Write SSE response headers + initial state, then hand fd to a keepalive thread. */
void handle_sse(int client_fd) {
    static const char header[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/event-stream\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: keep-alive\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n";
    write(client_fd, header, sizeof(header) - 1);

    char *buf = malloc(SSE_BUF_SIZE);
    if (buf) {
        char *json = buf + 6;
        int   json_len = sse_build_json(json, SSE_BUF_SIZE - 8);
        memcpy(buf, "data: ", 6);
        buf[6 + json_len]     = '\n';
        buf[6 + json_len + 1] = '\n';
        write(client_fd, buf, (size_t)(6 + json_len + 2));
        free(buf);
    }

    sse_client_add(client_fd);

    pthread_t tid;
    pthread_create(&tid, NULL, sse_client_thread, (void *)(intptr_t)client_fd);
    pthread_detach(tid);
}

/* Push current state to all connected SSE clients. Called after each visit insert. */
void sse_broadcast(void) {
    pthread_mutex_lock(&g_clients_mutex);
    int any = 0;
    for (int i = 0; i < SSE_MAX_CLIENTS; i++) {
        if (g_clients[i].fd >= 0) { any = 1; break; }
    }
    pthread_mutex_unlock(&g_clients_mutex);
    if (!any) return;

    char *buf = malloc(SSE_BUF_SIZE);
    if (!buf) return;

    char *json = buf + 6;
    int   json_len = sse_build_json(json, SSE_BUF_SIZE - 8);
    memcpy(buf, "data: ", 6);
    buf[6 + json_len]     = '\n';
    buf[6 + json_len + 1] = '\n';
    size_t total = (size_t)(6 + json_len + 2);

    pthread_mutex_lock(&g_clients_mutex);
    for (int i = 0; i < SSE_MAX_CLIENTS; i++) {
        if (g_clients[i].fd < 0) continue;
        if (write(g_clients[i].fd, buf, total) <= 0)
            g_clients[i].fd = -1;  /* client gone; thread will also detect and close */
    }
    pthread_mutex_unlock(&g_clients_mutex);

    free(buf);
}
