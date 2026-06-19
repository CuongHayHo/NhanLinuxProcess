/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/systeminfo/system_info.c
 * Purpose: Diagnostic host metrics systeminfo module.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/utsname.h>
#include "system_info.h"
#include "logger.h"

void system_info_run(void) {
    log_info("SYSTEM_INFO", "System information requested");
    
    printf("\n=== System Information (Sprint 1) ===\n");
    system_info_hostname();
    system_info_os();
    system_info_kernel();
    system_info_architecture();
    printf("=====================================\n");
}

int system_info_fetch_uname(void) {
    log_info("SYSTEM_INFO", "system_info_fetch_uname called");
    system_info_kernel();
    system_info_architecture();
    return 0;
}

int system_info_fetch_stats(void) {
    log_info("SYSTEM_INFO", "system_info_fetch_stats called (stub)");
    /* TODO: CPU/Memory/Disk stats are planned for subsequent sprints */
    printf("\n--- Advanced Metrics (Sprint 2 TODO) ---\n");
    printf("CPU Details:   [TODO - Not implemented]\n");
    printf("RAM/Uptime:    [TODO - Not implemented]\n");
    printf("Disk Metrics:  [TODO - Not implemented]\n");
    return 0;
}

int system_info_kernel(void) {
    struct utsname buf;
    if (uname(&buf) != 0) {
        log_error("SYSTEM_INFO", "API failure: uname() failed (errno %d)", errno);
        printf("Kernel Name:       Unknown\n");
        printf("Kernel Release:    Unknown\n");
        printf("Kernel Version:    Unknown\n");
        return -1;
    }
    printf("Kernel Name:       %s\n", buf.sysname);
    printf("Kernel Release:    %s\n", buf.release);
    printf("Kernel Version:    %s\n", buf.version);
    return 0;
}

int system_info_hostname(void) {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        log_error("SYSTEM_INFO", "API failure: gethostname() failed (errno %d)", errno);
        printf("Hostname:          Unknown\n");
        return -1;
    }
    printf("Hostname:          %s\n", hostname);
    return 0;
}

int system_info_os(void) {
    int fd;
    char buf[2048];
    ssize_t bytes_read;
    char os_name[256] = "Unknown OS";
    
    fd = open("/etc/os-release", O_RDONLY);
    if (fd < 0) {
        if (errno == ENOENT) {
            log_error("SYSTEM_INFO", "Missing file: /etc/os-release not found");
        } else if (errno == EACCES) {
            log_error("SYSTEM_INFO", "Permission denied: Cannot read /etc/os-release (errno %d)", errno);
        } else {
            log_error("SYSTEM_INFO", "API failure: open() on /etc/os-release failed (errno %d)", errno);
        }
        printf("Operating System:  Unknown (Failed to open /etc/os-release)\n");
        return -1;
    }
    
    bytes_read = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    
    if (bytes_read < 0) {
        log_error("SYSTEM_INFO", "API failure: read() on /etc/os-release failed (errno %d)", errno);
        printf("Operating System:  Unknown (Failed to read /etc/os-release)\n");
        return -1;
    }
    buf[bytes_read] = '\0';
    
    /* Parse for PRETTY_NAME= or NAME= */
    char* line = buf;
    char* next_line;
    int found = 0;
    
    while (line && *line != '\0') {
        next_line = strchr(line, '\n');
        if (next_line) {
            *next_line = '\0';
            next_line++;
        }
        
        if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
            char* val = line + 12;
            /* Strip quotes if present */
            if (*val == '"' || *val == '\'') {
                val++;
                char* end = strrchr(val, *val - 1);
                if (!end) {
                    end = strchr(val, '"');
                    if (!end) end = strchr(val, '\'');
                }
                if (end) *end = '\0';
            }
            strncpy(os_name, val, sizeof(os_name) - 1);
            os_name[sizeof(os_name) - 1] = '\0';
            found = 1;
            break;
        }
        line = next_line;
    }
    
    /* Fallback to NAME= if PRETTY_NAME not found */
    if (!found) {
        line = buf;
        while (line && *line != '\0') {
            next_line = strchr(line, '\n');
            if (next_line) {
                *next_line = '\0';
                next_line++;
            }
            if (strncmp(line, "NAME=", 5) == 0) {
                char* val = line + 5;
                if (*val == '"' || *val == '\'') {
                    val++;
                    char* end = strrchr(val, *val - 1);
                    if (!end) {
                        end = strchr(val, '"');
                        if (!end) end = strchr(val, '\'');
                    }
                    if (end) *end = '\0';
                }
                strncpy(os_name, val, sizeof(os_name) - 1);
                os_name[sizeof(os_name) - 1] = '\0';
                found = 1;
                break;
            }
            line = next_line;
        }
    }
    
    printf("Operating System:  %s\n", os_name);
    return 0;
}

int system_info_architecture(void) {
    struct utsname buf;
    if (uname(&buf) != 0) {
        log_error("SYSTEM_INFO", "API failure: uname() failed (errno %d)", errno);
        printf("Architecture:      Unknown\n");
        printf("Machine:           Unknown\n");
        printf("Node Name:         Unknown\n");
        return -1;
    }
    printf("Architecture:      %s\n", buf.machine);
    printf("Machine:           %s\n", buf.machine);
    printf("Node Name:         %s\n", buf.nodename);
    return 0;
}
