#include "ua_parser.h"
#include <string.h>

void ua_parse(const char *ua, char *browser, char *device_type, char *os) {
    if (!ua || ua[0] == '\0') {
        strncpy(browser,     "Other",   31);
        strncpy(device_type, "Desktop", 31);
        strncpy(os,          "Unknown", 31);
        return;
    }

    /* Device type — check Mobile/iPhone/iPod first, then tablet signals */
    if (strstr(ua, "Mobile") || strstr(ua, "iPhone") || strstr(ua, "iPod")) {
        strncpy(device_type, "Mobile", 31);
    } else if (strstr(ua, "iPad") || (strstr(ua, "Android") && !strstr(ua, "Mobile"))) {
        strncpy(device_type, "Tablet", 31);
    } else {
        strncpy(device_type, "Desktop", 31);
    }
    device_type[31] = '\0';

    /* OS — Android before Linux since Android UAs contain "Linux" */
    if (strstr(ua, "Android")) {
        strncpy(os, "Android", 31);
    } else if (strstr(ua, "iPhone") || strstr(ua, "iPad") || strstr(ua, "iOS")) {
        strncpy(os, "iOS", 31);
    } else if (strstr(ua, "Windows")) {
        strncpy(os, "Windows", 31);
    } else if (strstr(ua, "CrOS")) {
        strncpy(os, "ChromeOS", 31);
    } else if (strstr(ua, "Mac OS X")) {
        strncpy(os, "macOS", 31);
    } else if (strstr(ua, "Linux")) {
        strncpy(os, "Linux", 31);
    } else {
        strncpy(os, "Unknown", 31);
    }
    os[31] = '\0';

    /* Browser — Edge before Chrome since Edge UAs contain "Chrome" */
    if (strstr(ua, "Edg/")) {
        strncpy(browser, "Edge", 31);
    } else if (strstr(ua, "OPR/") || strstr(ua, "Opera/")) {
        strncpy(browser, "Opera", 31);
    } else if (strstr(ua, "Chrome/")) {
        strncpy(browser, "Chrome", 31);
    } else if (strstr(ua, "Firefox/")) {
        strncpy(browser, "Firefox", 31);
    } else if (strstr(ua, "Safari/")) {
        strncpy(browser, "Safari", 31);
    } else {
        strncpy(browser, "Other", 31);
    }
    browser[31] = '\0';
}
