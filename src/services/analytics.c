#include "analytics.h"
#include "../storage/db.h"
#include <string.h>

StatsResult analytics_get_stats(const char *country_filter, const char *city_filter, int dow, int hour,
    const char *browser_filter, const char *device_type_filter, const char *os_filter) {
    StatsResult result;
    memset(&result, 0, sizeof(result));

    result.total_visits     = db_get_total_visits(country_filter, city_filter, dow, hour, browser_filter, device_type_filter, os_filter);
    result.unique_visitors  = db_get_unique_visitors(country_filter, city_filter, dow, hour, browser_filter, device_type_filter, os_filter);
    result.unique_countries = db_get_unique_countries(country_filter, city_filter, dow, hour, browser_filter, device_type_filter, os_filter);
    result.top_len          = db_get_top_countries(result.top_countries,
                                                   result.top_counts, 50, dow, hour,
                                                   browser_filter, device_type_filter, os_filter);
    result.city_len         = db_get_top_cities(result.top_cities,
                                                result.city_counts, 50,
                                                country_filter, dow, hour,
                                                browser_filter, device_type_filter, os_filter);
    result.browser_len      = db_get_top_browsers(result.top_browsers,
                                                  result.browser_counts, 10,
                                                  country_filter, city_filter, dow, hour,
                                                  device_type_filter, os_filter);
    result.device_type_len  = db_get_device_breakdown(result.device_types,
                                                      result.device_type_counts, 5,
                                                      country_filter, city_filter, dow, hour,
                                                      browser_filter, os_filter);
    result.os_len           = db_get_top_os(result.top_os, result.os_counts, 10,
                                            country_filter, city_filter, dow, hour,
                                            browser_filter, device_type_filter);
    result.referrer_len     = db_get_top_referrers(result.top_referrers, result.referrer_counts, 10);
    result.avg_time_on_page = db_get_avg_time_on_page();

    if (country_filter && country_filter[0] != '\0') {
        strncpy(result.active_country, country_filter, 63);
        result.active_country[63] = '\0';
    }
    return result;
}
