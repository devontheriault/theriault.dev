#ifndef ANALYTICS_H
#define ANALYTICS_H

typedef struct {
    int  total_visits;
    int  unique_visitors;
    int  unique_countries;
    char top_countries[50][64];
    int  top_counts[50];
    int  top_len;
    char top_cities[50][64];
    int  city_counts[50];
    int  city_len;
    char active_country[64];
    char top_browsers[10][32];
    int  browser_counts[10];
    int  browser_len;
    char device_types[5][32];
    int  device_type_counts[5];
    int  device_type_len;
    char top_os[10][32];
    int  os_counts[10];
    int  os_len;
    char top_referrers[10][128];
    int  referrer_counts[10];
    int  referrer_len;
    double avg_time_on_page;
    char top_entry_pages[10][128];
    int  entry_page_counts[10];
    int  entry_page_len;
    char top_exit_pages[10][128];
    int  exit_page_counts[10];
    int  exit_page_len;
    char views_per_page[10][128];
    int  views_per_page_counts[10];
    int  views_per_page_len;
} StatsResult;

StatsResult analytics_get_stats(const char *country_filter, const char *city_filter, int dow, int hour, const char *browser_filter, const char *device_type_filter, const char *os_filter);

#endif /* ANALYTICS_H */
