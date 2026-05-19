#ifndef HASH_H
#define HASH_H

/* Compute a daily salted visitor ID from IP and User-Agent.
   out must be at least 65 bytes (64 hex chars + NUL). */
void hash_visitor_id(const char *ip, const char *ua, char out[65]);

#endif /* HASH_H */
