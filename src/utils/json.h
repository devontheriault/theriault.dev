#ifndef JSON_H
#define JSON_H

#include <stddef.h>

/* Escape a string for safe embedding in JSON (quotes, backslashes, control chars).
   Returns number of bytes written (excluding NUL). */
size_t json_string_escape(const char *in, char *out, size_t out_size);

#endif /* JSON_H */
