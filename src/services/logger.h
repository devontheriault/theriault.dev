#ifndef LOGGER_H
#define LOGGER_H

/* Record a visit. Returns the new row ID, or -1 on error. */
long long logger_record(const char *visitor_id, const char *country, const char *city, const char *user_agent, double lat, double lng, const char *referrer, const char *entry_page);

#endif /* LOGGER_H */
