#ifndef ANALYTICS_H
#define ANALYTICS_H

typedef struct {
    int  total_visits;
    int  unique_countries;
    char top_countries[50][64];
    int  top_counts[50];
    int  top_len;
    char top_cities[50][64];
    int  city_counts[50];
    int  city_len;
    char active_country[64];
} StatsResult;

StatsResult analytics_get_stats(const char *country_filter);

#endif /* ANALYTICS_H */
