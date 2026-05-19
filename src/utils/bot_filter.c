#define _POSIX_C_SOURCE 200809L
#include "bot_filter.h"
#include <string.h>
#include <strings.h>
#include <ctype.h>

/* Case-insensitive substring search */
static int contains_ci(const char *haystack, const char *needle) {
    size_t hlen = strlen(haystack);
    size_t nlen = strlen(needle);
    if (nlen == 0 || nlen > hlen) return 0;
    for (size_t i = 0; i <= hlen - nlen; i++) {
        size_t j;
        for (j = 0; j < nlen; j++) {
            if (tolower((unsigned char)haystack[i + j]) != tolower((unsigned char)needle[j]))
                break;
        }
        if (j == nlen) return 1;
    }
    return 0;
}

/* Known bot/scanner substrings found in User-Agent */
static const char *BOT_UA_PATTERNS[] = {
    "bot", "spider", "crawler", "scraper",
    "curl/", "wget/",
    "python-requests", "python/",
    "go-http-client",
    "java/", "jakarta",
    "libwww-perl", "perl/",
    "ruby/",
    "php/",
    "okhttp/",
    "apache-httpclient",
    "node-fetch", "node.js",
    "axios/",
    "got/",
    "httpx/",
    "scrapy/",
    "aiohttp/",
    "masscan",
    "zgrab",
    "nuclei",
    "nmap",
    "sqlmap",
    "nikto",
    NULL
};

/* File extensions that real browser SPA requests never carry */
static const char *BAD_PATH_EXTENSIONS[] = {
    ".php", ".asp", ".aspx", ".jsp",
    ".env", ".git", ".htaccess",
    ".xml", ".sh", ".cgi", ".pl",
    ".bak", ".sql", ".ini",
    ".config", ".conf",
    ".yml", ".yaml",
    ".log",
    NULL
};

int is_bot_request(const char *user_agent, const char *path) {
    /* Block empty or trivially short UA */
    if (!user_agent || strlen(user_agent) < 10)
        return 1;

    /* Block known bot UA strings */
    for (int i = 0; BOT_UA_PATTERNS[i]; i++) {
        if (contains_ci(user_agent, BOT_UA_PATTERNS[i]))
            return 1;
    }

    /* Block paths probing for file extensions real users never request on an SPA */
    if (path) {
        const char *dot = strrchr(path, '.');
        if (dot) {
            for (int i = 0; BAD_PATH_EXTENSIONS[i]; i++) {
                if (strcasecmp(dot, BAD_PATH_EXTENSIONS[i]) == 0)
                    return 1;
            }
        }
    }

    return 0;
}
