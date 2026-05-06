#ifndef DB_H
#define DB_H

int  db_init(void);
void db_close(void);
int  db_insert_visit(const char *ip, const char *country, const char *city, const char *user_agent);
int  db_get_total_visits(void);
int  db_get_unique_countries(void);
int  db_get_top_countries(char names[][64], int counts[], int max_n);
int  db_get_top_cities(char names[][64], int counts[], int max_n, const char *country_filter);

#endif /* DB_H */
