/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/time/time_mgr.c
 * Purpose: Time manager time inspection implementation and TUI submenu loop.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <errno.h>
#include "time_mgr.h"
#include "logger.h"

static int read_time_choice(void) {
    char input_buf[128];
    char* endptr;
    long val;

    if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
        return -1;
    }

    input_buf[strcspn(input_buf, "\n")] = '\0';
    if (strlen(input_buf) == 0) {
        return -1;
    }

    val = strtol(input_buf, &endptr, 10);
    if (*endptr != '\0') {
        return -1;
    }
    return (int)val;
}

static void time_menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF);
}

void time_mgr_run(void) {
    int choice;
    log_info("TIME", "Entering Time Manager");

    while (1) {
        printf("\n========================================\n");
        printf("              Time Manager\n");
        printf("========================================\n");
        printf("1. Show Current Time\n");
        printf("2. Time Zone (Future)\n");
        printf("3. Set Time (Preview)\n");
        printf("4. NTP Status (Future)\n");
        printf("0. Return\n");
        printf("========================================\n");
        printf("Select option: ");
        fflush(stdout);

        choice = read_time_choice();

        if (choice < 0) {
            continue;
        }

        if (choice > 4) {
            printf("\nInvalid input. Please choose a number between 0 and 4.\n");
            time_menu_pause();
            continue;
        }

        if (choice == 0) {
            log_info("TIME", "Leaving Time Manager");
            break;
        }

        if (choice >= 2 && choice <= 4) {
            printf("\nComing in a future sprint.\n");
            time_menu_pause();
            continue;
        }

        if (choice == 1) {
            time_mgr_show_current();
            time_menu_pause();
        }
    }
}

int time_mgr_show_current(void) {
    log_info("TIME", "Time inspection started");

    time_t t = time(NULL);
    if (t == (time_t)-1) {
        log_error("TIME", "API failure: time() failed (errno %d)", errno);
        printf("\nError: Failed to retrieve system time.\n");
        return -1;
    }
    log_info("TIME", "Current timestamp retrieved: %ld", (long)t);

    struct tm tm_local;
    if (localtime_r(&t, &tm_local) == NULL) {
        log_error("TIME", "API failure: localtime_r() failed (errno %d)", errno);
        printf("\nError: Failed to convert local time.\n");
        return -1;
    }

    struct tm tm_utc;
    if (gmtime_r(&t, &tm_utc) == NULL) {
        log_error("TIME", "API failure: gmtime_r() failed (errno %d)", errno);
        printf("\nError: Failed to convert UTC time.\n");
        return -1;
    }

    tzset();
    log_info("TIME", "Timezone detected: %s (Offset: %ld seconds)", 
             tm_local.tm_zone ? tm_local.tm_zone : "Unknown", 
             tm_local.tm_gmtoff);

    struct sysinfo s_info;
    int has_sysinfo = 0;
    if (sysinfo(&s_info) == 0) {
        has_sysinfo = 1;
    } else {
        log_error("TIME", "API failure: sysinfo() failed (errno %d)", errno);
    }

    // Format local date and time
    char local_date[64];
    char local_time[64];
    strftime(local_date, sizeof(local_date), "%Y-%m-%d", &tm_local);
    strftime(local_time, sizeof(local_time), "%H:%M:%S", &tm_local);

    // Format UTC time
    char utc_time[128];
    strftime(utc_time, sizeof(utc_time), "%Y-%m-%d %H:%M:%S UTC", &tm_utc);

    // Format timezone offset
    long offset_sec = tm_local.tm_gmtoff;
    char tz_offset_buf[32];
    long abs_offset = offset_sec >= 0 ? offset_sec : -offset_sec;
    snprintf(tz_offset_buf, sizeof(tz_offset_buf), "UTC%+03d:%02d", 
             (int)(offset_sec / 3600), (int)((abs_offset % 3600) / 60));

    printf("\n========================================\n");
    printf("Time Information\n");
    printf("----------------------------------------\n");
    printf("Local Date:     %s\n", local_date);
    printf("Local Time:     %s\n", local_time);
    printf("UTC Time:       %s\n", utc_time);
    printf("Unix Timestamp: %ld\n", (long)t);
    printf("Time Zone:      %s (%s)\n", tm_local.tm_zone ? tm_local.tm_zone : "N/A", tz_offset_buf);

    if (has_sysinfo) {
        long days = s_info.uptime / 86400;
        long hours = (s_info.uptime % 86400) / 3600;
        long mins = (s_info.uptime % 3600) / 60;
        long secs = s_info.uptime % 60;
        printf("Uptime:         %ld days, %02ld:%02ld:%02ld\n", days, hours, mins, secs);

        time_t boot_t = t - s_info.uptime;
        struct tm tm_boot;
        if (localtime_r(&boot_t, &tm_boot) != NULL) {
            char boot_time_str[128];
            strftime(boot_time_str, sizeof(boot_time_str), "%Y-%m-%d %H:%M:%S", &tm_boot);
            printf("Boot Time:      %s\n", boot_time_str);
        } else {
            printf("Boot Time:      N/A\n");
        }
    } else {
        printf("Uptime:         N/A (sysinfo failed)\n");
        printf("Boot Time:      N/A (sysinfo failed)\n");
    }
    printf("========================================\n");

    return 0;
}
