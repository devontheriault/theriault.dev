#include "json.h"
#include <string.h>

size_t json_string_escape(const char *in, char *out, size_t out_size) {
    if (!in || !out || out_size == 0) return 0;

    size_t written = 0;
    for (const char *p = in; *p && written + 2 < out_size; p++) {
        unsigned char c = (unsigned char)*p;
        if (c == '"' || c == '\\') {
            if (written + 3 >= out_size) break;
            out[written++] = '\\';
            out[written++] = (char)c;
        } else if (c < 0x20) {
            /* Escape control characters as \uXXXX */
            if (written + 7 >= out_size) break;
            out[written++] = '\\';
            out[written++] = 'u';
            out[written++] = '0';
            out[written++] = '0';
            out[written++] = "0123456789abcdef"[(c >> 4) & 0xf];
            out[written++] = "0123456789abcdef"[c & 0xf];
        } else {
            out[written++] = (char)c;
        }
    }
    out[written] = '\0';
    return written;
}
