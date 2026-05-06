#ifndef GEO_H
#define GEO_H

/* Look up country and city for the given IP address.
   country_out and city_out must be at least 64 bytes each.
   Returns 0 on success, -1 on failure (outputs still filled with "Unknown"). */
int geo_lookup(const char *ip, char *country_out, char *city_out);

#endif /* GEO_H */
