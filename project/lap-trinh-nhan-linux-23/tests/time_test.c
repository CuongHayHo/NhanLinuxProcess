/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/time_test.c
 * Purpose: Diagnostic unit tests for Time Manager.
 */

#include <stdio.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <assert.h>
#include "time_mgr.h"

int main(void) {
    printf("Starting Time Manager Module - Sprint 1 (Time Inspection) test program...\n\n");

    printf("--- Test 1: POSIX Time Inspection API Call ---\n");
    int res = time_mgr_show_current();
    printf("time_mgr_show_current() returned: %d\n", res);
    assert(res == 0);

    printf("\n--- Test 2: Unit Validation of Time Metrics ---\n");
    // Verify time()
    time_t t1 = time(NULL);
    assert(t1 != (time_t)-1);
    printf("POSIX time() retrieved successfully: %ld\n", (long)t1);

    // Verify local time formatting
    struct tm tm_local;
    assert(localtime_r(&t1, &tm_local) != NULL);
    char date_buf[64];
    size_t written = strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", &tm_local);
    assert(written > 0);
    printf("Formatted Local Date: %s (Length: %zu)\n", date_buf, written);

    // Verify UTC time formatting
    struct tm tm_utc;
    assert(gmtime_r(&t1, &tm_utc) != NULL);
    char utc_buf[64];
    written = strftime(utc_buf, sizeof(utc_buf), "%H:%M:%S UTC", &tm_utc);
    assert(written > 0);
    printf("Formatted UTC Time: %s\n", utc_buf);

    // Verify timezone fields
    tzset();
    printf("Detected Timezone Zone Name: %s\n", tm_local.tm_zone ? tm_local.tm_zone : "N/A");
    printf("Detected Timezone UTC Offset: %ld seconds\n", tm_local.tm_gmtoff);

    // Verify uptime
    struct sysinfo s_info;
    if (sysinfo(&s_info) == 0) {
        printf("System Uptime: %ld seconds\n", s_info.uptime);
        assert(s_info.uptime >= 0);
    } else {
        printf("sysinfo() call failed.\n");
    }

    printf("\nTime Manager Module tests completed successfully.\n");
    return 0;
}
