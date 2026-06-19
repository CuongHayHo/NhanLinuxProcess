/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/process/process_mgr.c
 * Purpose: Process management and process lifecycle demonstrations.
 */

#define _GNU_SOURCE /* Required for strcasestr in glibc */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <signal.h>
#include "process_mgr.h"
#include "logger.h"
#include "process_mgr_internal.h"
#include "demo/process_demo.h"

/* Static helper prototypes */
static int parse_proc_stat(pid_t pid, proc_info_t* info);
static unsigned long parse_proc_status_memory(pid_t pid);
static void get_proc_cmdline(pid_t pid, char* cmd_out, size_t max_len);

static int read_process_choice(void) {
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

static void process_menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF);
}

void process_mgr_run(void) {
    int choice;
    log_info("PROCESS", "Process manager interactive loop started.");

    while (1) {
        printf("\n========================================\n");
        printf("           Process Manager\n");
        printf("========================================\n");
        printf("1. List Processes\n");
        printf("2. Search Process\n");
        printf("3. Send Signal\n");
        printf("4. Change Priority\n");
        printf("----------------------------------------\n");
        printf("5. Fork Demo\n");
        printf("6. Exec Demo\n");
        printf("7. Wait Demo\n");
        printf("8. Zombie Demo\n");
        printf("9. Orphan Demo\n");
        printf("10. Daemon Demo\n");
        printf("11. Return\n");
        printf("========================================\n");
        printf("Select option: ");
        fflush(stdout);

        choice = read_process_choice();

        if (choice < 1 || choice > 11) {
            printf("\nInvalid input. Please choose a number between 1 and 11.\n");
            process_menu_pause();
            continue;
        }

        if (choice == 11) {
            log_info("PROCESS", "Process manager interactive loop ended.");
            break;
        }

        switch (choice) {
            case 1:
                process_mgr_list();
                process_menu_pause();
                break;
            case 2: {
                char query[128];
                printf("Enter PID or Process Name to search: ");
                fflush(stdout);
                if (fgets(query, sizeof(query), stdin) != NULL) {
                    query[strcspn(query, "\n")] = '\0';
                    process_mgr_search(query);
                }
                process_menu_pause();
                break;
            }
            case 3: {
                int pid_in = 0, sig_in = 0;
                char buf[128];
                printf("Enter target PID: ");
                fflush(stdout);
                if (fgets(buf, sizeof(buf), stdin) != NULL) {
                    pid_in = atoi(buf);
                }
                printf("Enter signal number (e.g. 15 for SIGTERM, 9 for SIGKILL, 19 for SIGSTOP, 18 for SIGCONT): ");
                fflush(stdout);
                if (fgets(buf, sizeof(buf), stdin) != NULL) {
                    sig_in = atoi(buf);
                }
                process_mgr_send_signal(pid_in, sig_in);
                process_menu_pause();
                break;
            }
            case 4: {
                int pid_in = 0, nice_in = 0;
                char buf[128];
                printf("Enter target PID: ");
                fflush(stdout);
                if (fgets(buf, sizeof(buf), stdin) != NULL) {
                    pid_in = atoi(buf);
                }
                printf("Enter new nice value (-20 to 19): ");
                fflush(stdout);
                if (fgets(buf, sizeof(buf), stdin) != NULL) {
                    nice_in = atoi(buf);
                }
                process_mgr_set_priority(pid_in, nice_in);
                process_menu_pause();
                break;
            }
            case 5:
                fork_demo_run();
                process_menu_pause();
                break;
            case 6:
                exec_demo_run();
                process_menu_pause();
                break;
            case 7:
                wait_demo_run();
                process_menu_pause();
                break;
            case 8:
                zombie_demo_run();
                process_menu_pause();
                break;
            case 9:
                orphan_demo_run();
                process_menu_pause();
                break;
            case 10:
                daemon_demo_run();
                process_menu_pause();
                break;
        }
    }
}

int process_mgr_list(void) {
    DIR* dir;
    struct dirent* entry;
    proc_info_t info;
    int count = 0;

    dir = opendir("/proc");
    if (dir == NULL) {
        log_error("PROCESS", "Failed to open /proc directory");
        return -1;
    }

    log_info("PROCESS", "Process list requested");

    printf("%-8s %-8s %-6s %-12s %-8s %-30s\n", "PID", "PPID", "State", "Memory", "Priority", "Name/Command");
    printf("-----------------------------------------------------------------------------------------\n");

    while ((entry = readdir(dir)) != NULL) {
        pid_t pid;
        char* endptr;

        /* Ignore non-numeric directories in /proc */
        pid = (pid_t)strtol(entry->d_name, &endptr, 10);
        if (*endptr != '\0') {
            continue;
        }

        /* Attempt to parse stat. Skip failures due to race condition exits or permissions */
        if (parse_proc_stat(pid, &info) == 0) {
            char cmdline[256];
            get_proc_cmdline(pid, cmdline, sizeof(cmdline));

            /* Use cmdline if available, fallback to short comm name */
            const char* name_to_show = (strlen(cmdline) > 0) ? cmdline : info.name;

            printf("%-8d %-8d %-6c %-12lu KB %-8ld %-30s\n", 
                   info.pid, 
                   info.ppid, 
                   info.state, 
                   info.memory_size / 1024, 
                   info.priority, 
                   name_to_show);
            count++;
        }
    }

    printf("-----------------------------------------------------------------------------------------\n");
    closedir(dir);
    
    log_info("PROCESS", "Number of processes found: %d", count);
    return count;
}

int process_mgr_search(const char* query) {
    if (query == NULL || strlen(query) == 0) {
        log_error("PROCESS", "Search request failed: empty query");
        return -1;
    }

    log_info("PROCESS", "Search request: '%s'", query);

    /* Determine if query is numeric (PID search) */
    int is_numeric = 1;
    for (int i = 0; query[i] != '\0'; i++) {
        if (query[i] < '0' || query[i] > '9') {
            is_numeric = 0;
            break;
        }
    }

    if (is_numeric) {
        pid_t target_pid = (pid_t)strtol(query, NULL, 10);
        proc_info_t info;
        if (parse_proc_stat(target_pid, &info) == 0) {
            char cmdline[256];
            get_proc_cmdline(target_pid, cmdline, sizeof(cmdline));

            printf("Process Information for PID: %d\n", target_pid);
            printf("----------------------------------------\n");
            printf("PID:        %d\n", info.pid);
            printf("PPID:       %d\n", info.ppid);
            printf("Name:       %s\n", info.name);
            printf("State:      %c\n", info.state);
            printf("Priority:   %ld\n", info.priority);
            printf("Memory:     %lu KB\n", info.memory_size / 1024);
            printf("Executable: %s\n", (strlen(cmdline) > 0) ? cmdline : "N/A");
            printf("----------------------------------------\n");
            
            log_info("PROCESS", "Matched processes: 1 (PID search)");
            return 1;
        } else {
            printf("Process with PID %d not found.\n", target_pid);
            log_warning("PROCESS", "Search failure: PID %d not found", target_pid);
            return 0;
        }
    } else {
        /* Name search: scan /proc */
        DIR* dir;
        struct dirent* entry;
        int count = 0;

        dir = opendir("/proc");
        if (dir == NULL) {
            log_error("PROCESS", "Failed to open /proc directory for searching");
            return -1;
        }

        printf("%-8s %-8s %-6s %-12s %-8s %-30s\n", "PID", "PPID", "State", "Memory", "Priority", "Name/Command");
        printf("-----------------------------------------------------------------------------------------\n");

        while ((entry = readdir(dir)) != NULL) {
            pid_t pid;
            char* endptr;
            proc_info_t info;

            pid = (pid_t)strtol(entry->d_name, &endptr, 10);
            if (*endptr != '\0') {
                continue;
            }

            if (parse_proc_stat(pid, &info) == 0) {
                char cmdline[256];
                get_proc_cmdline(pid, cmdline, sizeof(cmdline));

                /* Case-insensitive substring match on short comm name or full cmdline */
                if (strcasestr(info.name, query) != NULL || (strlen(cmdline) > 0 && strcasestr(cmdline, query) != NULL)) {
                    const char* name_to_show = (strlen(cmdline) > 0) ? cmdline : info.name;
                    printf("%-8d %-8d %-6c %-12lu KB %-8ld %-30s\n", 
                           info.pid, 
                           info.ppid, 
                           info.state, 
                           info.memory_size / 1024, 
                           info.priority, 
                           name_to_show);
                    count++;
                }
            }
        }

        printf("-----------------------------------------------------------------------------------------\n");
        closedir(dir);

        if (count > 0) {
            log_info("PROCESS", "Matched processes: %d (Name search)", count);
        } else {
            printf("No matching processes found for query: '%s'\n", query);
            log_warning("PROCESS", "Search failure: Name query '%s' matched 0 processes", query);
        }

        return count;
    }
}

static const char* get_signal_name(int sig) {
    switch (sig) {
        case SIGTERM: return "SIGTERM";
        case SIGKILL: return "SIGKILL";
        case SIGSTOP: return "SIGSTOP";
        case SIGCONT: return "SIGCONT";
        default:      return "UNKNOWN";
    }
}

int process_mgr_send_signal(pid_t pid, int sig) {
    proc_info_t info;
    char proc_name[256] = "N/A";

    /* Retrieve process name if possible */
    if (pid > 0 && parse_proc_stat(pid, &info) == 0) {
        strncpy(proc_name, info.name, sizeof(proc_name) - 1);
        proc_name[sizeof(proc_name) - 1] = '\0';
    }

    if (pid <= 0) {
        log_error("PROCESS", "Invalid PID: %d", pid);
        printf("PID:              %d\n", pid);
        printf("Process Name:     %s\n", proc_name);
        printf("Signal Sent:      %s (%d)\n", get_signal_name(sig), sig);
        printf("Operation Result: Error: Invalid PID (Negative/Zero)\n");
        return -1;
    }

    if (kill(pid, sig) != 0) {
        if (errno == ESRCH) {
            log_error("PROCESS", "Invalid PID: %d (not found)", pid);
            printf("PID:              %d\n", pid);
            printf("Process Name:     %s\n", proc_name);
            printf("Signal Sent:      %s (%d)\n", get_signal_name(sig), sig);
            printf("Operation Result: Error: Process not found (ESRCH)\n");
        } else if (errno == EPERM) {
            log_error("PROCESS", "Permission denied for PID %d", pid);
            printf("PID:              %d\n", pid);
            printf("Process Name:     %s\n", proc_name);
            printf("Signal Sent:      %s (%d)\n", get_signal_name(sig), sig);
            printf("Operation Result: Error: Permission denied (EPERM)\n");
        } else if (errno == EINVAL) {
            log_error("PROCESS", "Invalid signal %d", sig);
            printf("PID:              %d\n", pid);
            printf("Process Name:     %s\n", proc_name);
            printf("Signal Sent:      %s (%d)\n", get_signal_name(sig), sig);
            printf("Operation Result: Error: Invalid signal (EINVAL)\n");
        } else {
            log_error("PROCESS", "Signal failed for PID %d (errno %d)", pid, errno);
            printf("PID:              %d\n", pid);
            printf("Process Name:     %s\n", proc_name);
            printf("Signal Sent:      %s (%d)\n", get_signal_name(sig), sig);
            printf("Operation Result: Error: Failed (errno %d)\n", errno);
        }
        return -1;
    }

    log_info("PROCESS", "Signal sent: %s (%d) to PID %d", get_signal_name(sig), sig, pid);
    printf("PID:              %d\n", pid);
    printf("Process Name:     %s\n", proc_name);
    printf("Signal Sent:      %s (%d)\n", get_signal_name(sig), sig);
    printf("Operation Result: Success\n");
    return 0;
}

int process_mgr_set_priority(pid_t pid, int nice_val) {
    proc_info_t info;
    char proc_name[256] = "N/A";
    int current_nice = 0;

    /* Retrieve process name if possible */
    if (pid > 0 && parse_proc_stat(pid, &info) == 0) {
        strncpy(proc_name, info.name, sizeof(proc_name) - 1);
        proc_name[sizeof(proc_name) - 1] = '\0';
    }

    if (pid <= 0) {
        log_error("PROCESS", "Invalid PID: %d", pid);
        printf("PID:              %d\n", pid);
        printf("Process Name:     %s\n", proc_name);
        printf("Current Nice:     N/A\n");
        printf("New Nice:         %d\n", nice_val);
        printf("Operation Result: Error: Invalid PID (Negative/Zero)\n");
        return -1;
    }

    /* Clear errno before calling getpriority to distinguish -1 success from failure */
    errno = 0;
    current_nice = getpriority(PRIO_PROCESS, pid);
    if (current_nice == -1 && errno != 0) {
        if (errno == ESRCH) {
            log_error("PROCESS", "Invalid PID: %d (not found)", pid);
            printf("PID:              %d\n", pid);
            printf("Process Name:     %s\n", proc_name);
            printf("Current Nice:     N/A\n");
            printf("New Nice:         %d\n", nice_val);
            printf("Operation Result: Error: Process not found (ESRCH)\n");
        } else if (errno == EPERM) {
            log_error("PROCESS", "Permission denied for PID %d", pid);
            printf("PID:              %d\n", pid);
            printf("Process Name:     %s\n", proc_name);
            printf("Current Nice:     N/A\n");
            printf("New Nice:         %d\n", nice_val);
            printf("Operation Result: Error: Permission denied (EPERM)\n");
        } else {
            log_error("PROCESS", "Priority query failed for PID %d (errno %d)", pid, errno);
            printf("PID:              %d\n", pid);
            printf("Process Name:     %s\n", proc_name);
            printf("Current Nice:     N/A\n");
            printf("New Nice:         %d\n", nice_val);
            printf("Operation Result: Error: Priority query failed (errno %d)\n", errno);
        }
        return -1;
    }

    if (setpriority(PRIO_PROCESS, pid, nice_val) != 0) {
        if (errno == EPERM || errno == EACCES) {
            log_error("PROCESS", "Permission denied for PID %d", pid);
            printf("PID:              %d\n", pid);
            printf("Process Name:     %s\n", proc_name);
            printf("Current Nice:     %d\n", current_nice);
            printf("New Nice:         %d\n", nice_val);
            printf("Operation Result: Error: Permission denied (EPERM/EACCES)\n");
        } else if (errno == EINVAL) {
            log_error("PROCESS", "Invalid nice value %d", nice_val);
            printf("PID:              %d\n", pid);
            printf("Process Name:     %s\n", proc_name);
            printf("Current Nice:     %d\n", current_nice);
            printf("New Nice:         %d\n", nice_val);
            printf("Operation Result: Error: Invalid nice value (EINVAL)\n");
        } else if (errno == ESRCH) {
            log_error("PROCESS", "Invalid PID: %d (not found)", pid);
            printf("PID:              %d\n", pid);
            printf("Process Name:     %s\n", proc_name);
            printf("Current Nice:     %d\n", current_nice);
            printf("New Nice:         %d\n", nice_val);
            printf("Operation Result: Error: Process not found (ESRCH)\n");
        } else {
            log_error("PROCESS", "Priority change failed for PID %d (errno %d)", pid, errno);
            printf("PID:              %d\n", pid);
            printf("Process Name:     %s\n", proc_name);
            printf("Current Nice:     %d\n", current_nice);
            printf("New Nice:         %d\n", nice_val);
            printf("Operation Result: Error: Priority change failed (errno %d)\n", errno);
        }
        return -1;
    }

    log_info("PROCESS", "Priority changed: PID %d nice from %d to %d", pid, current_nice, nice_val);
    printf("PID:              %d\n", pid);
    printf("Process Name:     %s\n", proc_name);
    printf("Current Nice:     %d\n", current_nice);
    printf("New Nice:         %d\n", nice_val);
    printf("Operation Result: Success\n");
    return 0;
}

void process_mgr_demo_zombie(void) {
    log_info("PROCESS", "process_mgr_demo_zombie called (stub).");
    /* TODO: Create zombie process using fork() and quick exit child (Sprint 4) */
    printf("Zombie Process Demo: Stub. (TODO)\n");
}

void process_mgr_demo_orphan(void) {
    log_info("PROCESS", "process_mgr_demo_orphan called (stub).");
    /* TODO: Create orphan process by exiting parent immediately (Sprint 4) */
    printf("Orphan Process Demo: Stub. (TODO)\n");
}

/* ==========================================================
 * Static helper implementations
 * ========================================================== */

static int parse_proc_stat(pid_t pid, proc_info_t* info) {
    char path[128];
    char buf[1024];
    int fd;
    ssize_t bytes;
    char *p, *start;

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1; /* Skip: process might have finished or permission denied */
    }

    bytes = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (bytes <= 0) {
        return -1;
    }
    buf[bytes] = '\0';

    /* Parse comm name inside parentheses (handles spaces in names) */
    p = strrchr(buf, ')');
    start = strchr(buf, '(');
    if (!p || !start || start >= p) {
        return -1;
    }

    /* Extract PID and comm name */
    info->pid = pid;
    size_t name_len = p - (start + 1);
    if (name_len >= sizeof(info->name)) {
        name_len = sizeof(info->name) - 1;
    }
    memcpy(info->name, start + 1, name_len);
    info->name[name_len] = '\0';

    /* State is the char right after the space following ')' */
    if (*(p + 1) == ' ' && *(p + 2) != '\0') {
        info->state = *(p + 2);
        
        /* Scan PPID and Priority fields starting after the state char */
        long ppid_val = 0;
        long priority_val = 0;
        
        /* sscanf format description: 
         * %ld: PPID (1st field after state)
         * %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*u %*u: skip next 13 fields
         * %ld: Priority (15th field after state)
         */
        int scanned = sscanf(p + 3, "%ld %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*u %*u %ld", 
                             &ppid_val, &priority_val);
        if (scanned >= 1) {
            info->ppid = (pid_t)ppid_val;
            info->priority = priority_val;
        } else {
            info->ppid = 0;
            info->priority = 0;
        }
    } else {
        return -1;
    }

    /* Query RSS memory size from status */
    info->memory_size = parse_proc_status_memory(pid);

    return 0;
}

static unsigned long parse_proc_status_memory(pid_t pid) {
    char path[128];
    char buf[4096];
    int fd;
    ssize_t bytes;
    char* p;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        return 0;
    }

    bytes = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (bytes <= 0) {
        return 0;
    }
    buf[bytes] = '\0';

    p = strstr(buf, "VmRSS:");
    if (p) {
        unsigned long rss_val = 0;
        if (sscanf(p + 6, "%lu", &rss_val) == 1) {
            return rss_val * 1024; /* Convert kB to bytes */
        }
    }
    return 0;
}

static void get_proc_cmdline(pid_t pid, char* cmd_out, size_t max_len) {
    char path[128];
    char buf[512];
    int fd;
    ssize_t bytes;

    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        cmd_out[0] = '\0';
        return;
    }

    bytes = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (bytes <= 0) {
        cmd_out[0] = '\0';
        return;
    }

    /* Replace internal null separations with spaces */
    for (ssize_t i = 0; i < bytes - 1; i++) {
        if (buf[i] == '\0') {
            buf[i] = ' ';
        }
    }
    buf[bytes - 1] = '\0';

    strncpy(cmd_out, buf, max_len - 1);
    cmd_out[max_len - 1] = '\0';
}


