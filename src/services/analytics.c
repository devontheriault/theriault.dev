#include "analytics.h"
#include "../storage/db.h"
#include <string.h>

StatsResult analytics_get_stats(const char *country_filter) {
    StatsResult result;
    memset(&result, 0, sizeof(result));

    result.total_visits     = db_get_total_visits();
    result.unique_countries = db_get_unique_countries();
    result.top_len          = db_get_top_countries(result.top_countries,
                                                   result.top_counts, 5);
    result.city_len         = db_get_top_cities(result.top_cities,
                                                result.city_counts, 10,
                                                country_filter);
    if (country_filter && country_filter[0] != '\0') {
        strncpy(result.active_country, country_filter, 63);
        result.active_country[63] = '\0';
    }
    return result;
}
