#ifndef NET_H
#define NET_H

/* Decode URL percent-encoding in-place (%20 → space, + → space). */
void url_decode(char *str);

/* Extract the real client IP from the raw HTTP request.
   Checks X-Forwarded-For header first, falls back to peer_addr.
   ip_out must be at least 64 bytes. */
void net_extract_ip(const char *raw_request, const char *peer_addr, char *ip_out);

/* Extract the User-Agent header value from the raw HTTP request.
   ua_out must be at least 256 bytes. */
void net_extract_user_agent(const char *raw_request, char *ua_out);

#endif /* NET_H */
