#ifndef ANALYTICS_H
#define ANALYTICS_H

typedef struct {
    int  total_visits;
    int  unique_countries;
    char top_countries[5][64];
    int  top_counts[5];
    int  top_len;
    char top_cities[10][64];
    int  city_counts[10];
    int  city_len;
    char active_country[64];
} StatsResult;

StatsResult analytics_get_stats(const char *country_filter);

#endif /* ANALYTICS_H */
