#ifndef LOGGER_H
#define LOGGER_H

/* Record a visit to the database. */
int logger_record(const char *ip, const char *country, const char *city, const char *user_agent);

#endif /* LOGGER_H */
