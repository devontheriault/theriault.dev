#ifndef DB_H
#define DB_H

int  db_init(void);
void db_close(void);
int    db_insert_visit(const char *ip, const char *country, const char *city, const char *user_agent, double lat, double lng);
int    db_update_visit_duration(const char *ip, const char *user_agent, int seconds);
double db_get_avg_time_on_page(void);
int  db_get_total_visits(const char *country_filter, const char *city_filter, int dow, int hour, const char *browser_filter, const char *device_type_filter, const char *os_filter);
int  db_get_unique_visitors(const char *country_filter, const char *city_filter, int dow, int hour, const char *browser_filter, const char *device_type_filter, const char *os_filter);
int  db_get_unique_countries(const char *country_filter, const char *city_filter, int dow, int hour, const char *browser_filter, const char *device_type_filter, const char *os_filter);
int  db_get_top_countries(char names[][64], int counts[], int max_n, int dow, int hour, const char *browser_filter, const char *device_type_filter, const char *os_filter);
int  db_get_all_countries(char names[][64], int counts[], int max_n);
int  db_get_top_cities(char names[][64], int counts[], int max_n, const char *country_filter, int dow, int hour, const char *browser_filter, const char *device_type_filter, const char *os_filter);
int  db_get_city_globe_data(char cities[][64], char countries[][64], int counts[], double lats[], double lngs[], int max_n);
int  db_get_heatmap_data(int counts[7][24], const char *country_filter, const char *city_filter, const char *browser_filter, const char *device_type_filter, const char *os_filter); /* returns max count */
int  db_get_top_browsers(char names[][32], int counts[], int max_n, const char *country_filter, const char *city_filter, int dow, int hour, const char *device_type_filter, const char *os_filter);
int  db_get_device_breakdown(char names[][32], int counts[], int max_n, const char *country_filter, const char *city_filter, int dow, int hour, const char *browser_filter, const char *os_filter);
int  db_get_top_os(char names[][32], int counts[], int max_n, const char *country_filter, const char *city_filter, int dow, int hour, const char *browser_filter, const char *device_type_filter);

#endif /* DB_H */
