/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/kernel/kernel_mgr.c
 * Purpose: Kernel module user space integration manager and submenu.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <errno.h>
#include "kernel_mgr.h"
#include "logger.h"
#include "menu.h"

#define MAX_LOG_LINES 20
#define MAX_LINE_LEN 512

static int read_kernel_choice(void) {
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

static void kernel_menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Custom case-insensitive search */
static int contains_keyword(const char *str, const char *keyword) {
    if (!str || !keyword) return 0;
    size_t str_len = strlen(str);
    size_t kw_len = strlen(keyword);
    if (kw_len > str_len) return 0;
    for (size_t i = 0; i <= str_len - kw_len; i++) {
        size_t j;
        for (j = 0; j < kw_len; j++) {
            char c1 = str[i + j];
            char c2 = keyword[j];
            if (c1 >= 'A' && c1 <= 'Z') c1 = c1 - 'A' + 'a';
            if (c2 >= 'A' && c2 <= 'Z') c2 = c2 - 'A' + 'a';
            if (c1 != c2) break;
        }
        if (j == kw_len) return 1;
    }
    return 0;
}

/* Direct process execution helper without shell */
static int run_command_direct(const char *wdir, const char *cmd, char *const argv[], int *exit_code) {
    pid_t pid = fork();
    if (pid == -1) {
        log_error("KERNEL", "fork failed: %s", strerror(errno));
        return -1;
    }
    if (pid == 0) {
        if (wdir) {
            if (chdir(wdir) == -1) {
                exit(errno);
            }
        }
        execvp(cmd, argv);
        exit(errno);
    }
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        log_error("KERNEL", "waitpid failed: %s", strerror(errno));
        return -1;
    }
    if (WIFEXITED(status)) {
        if (exit_code) {
            *exit_code = WEXITSTATUS(status);
        }
        return 0;
    }
    return -1;
}

int is_module_loaded(void) {
    FILE *fp = fopen("/proc/modules", "r");
    if (!fp) {
        return 0;
    }
    char line[256];
    int loaded = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "system_monitor ", 15) == 0) {
            loaded = 1;
            break;
        }
    }
    fclose(fp);
    return loaded;
}

int is_module_loaded_via_lsmod(void) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return 0;
    }
    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return 0;
    }
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull != -1) {
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        close(pipefd[1]);
        char *argv[] = {"lsmod", NULL};
        execvp("lsmod", argv);
        exit(errno);
    }
    close(pipefd[1]);
    
    char buf[4096];
    ssize_t n;
    int found = 0;
    while ((n = read(pipefd[0], buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        if (strstr(buf, "system_monitor") != NULL) {
            found = 1;
        }
    }
    close(pipefd[0]);
    waitpid(pid, NULL, 0);
    return found;
}

int kernel_mgr_load_module_internal(int use_sudo) {
    int exit_code = 0;
    log_info("KERNEL", "Module load requested");

    if (is_module_loaded() || is_module_loaded_via_lsmod()) {
        printf("Module is already loaded.\n");
        return 0;
    }

    // Determine whether system_monitor.ko already exists
    int ko_exists = (access("kernel/system_monitor/system_monitor.ko", F_OK) == 0);
    if (!ko_exists) {
        log_info("KERNEL", "Module build started");
        printf("system_monitor.ko not found. Auto-building module...\n");
        
        char *make_argv[] = {"make", NULL};
        int res = run_command_direct("kernel/system_monitor", "make", make_argv, &exit_code);
        if (res != 0 || exit_code != 0) {
            log_error("KERNEL", "Build failed");
            printf("Build failure: make failed (exit code %d).\n", exit_code);
            printf("Please ensure kernel headers are installed (e.g. linux-headers-$(uname -r)).\n");
            return -1;
        }
        
        // Double check after building
        if (access("kernel/system_monitor/system_monitor.ko", F_OK) != 0) {
            log_error("KERNEL", "Build failed");
            printf("Error: Missing system_monitor.ko after build.\n");
            return -1;
        }
    }

    // Execute insmod
    if (use_sudo) {
        char *insmod_argv[] = {"sudo", "insmod", "system_monitor.ko", NULL};
        int res = run_command_direct("kernel/system_monitor", "sudo", insmod_argv, &exit_code);
        if (res != 0 || exit_code != 0) {
            log_error("KERNEL", "Permission denied");
            printf("Permission denied: Failed to load module (exit code %d).\n", exit_code);
            return -1;
        }
    } else {
        char *insmod_argv[] = {"insmod", "system_monitor.ko", NULL};
        int res = run_command_direct("kernel/system_monitor", "insmod", insmod_argv, &exit_code);
        if (res != 0 || exit_code != 0) {
            log_error("KERNEL", "Permission denied");
            printf("Permission denied: Failed to load module (exit code %d).\n", exit_code);
            return -1;
        }
    }

    // Verify /proc/sysmgr exists
    if (access("/proc/sysmgr", F_OK) == 0) {
        log_info("KERNEL", "Module loaded");
        printf("Kernel module loaded successfully.\n");
        return 0;
    } else {
        log_error("KERNEL", "Build failed");
        printf("Error: /proc/sysmgr is missing after loading.\n");
        return -1;
    }
}

int kernel_mgr_load_module(void) {
    return kernel_mgr_load_module_internal(1);
}

int kernel_mgr_load_module_no_sudo(void) {
    return kernel_mgr_load_module_internal(0);
}

int kernel_mgr_unload_module(void) {
    int exit_code = 0;
    log_info("KERNEL", "Module unload requested");
    
    if (!is_module_loaded() && !is_module_loaded_via_lsmod()) {
        printf("Module is not currently loaded.\n");
        return 0;
    }

    char *rmmod_argv[] = {"sudo", "rmmod", "system_monitor", NULL};
    int res = run_command_direct(NULL, "sudo", rmmod_argv, &exit_code);
    if (res != 0 || exit_code != 0) {
        log_error("KERNEL", "Permission denied");
        printf("Permission denied: Failed to unload module (exit code %d).\n", exit_code);
        return -1;
    }

    // Verify /proc/sysmgr no longer exists
    if (access("/proc/sysmgr", F_OK) != 0) {
        log_info("KERNEL", "Module unloaded");
        printf("Kernel module unloaded successfully.\n");
        return 0;
    } else {
        printf("Error: /proc/sysmgr still exists after unloading.\n");
        return -1;
    }
}

int kernel_mgr_show_status(void) {
    log_info("KERNEL", "Module status checked");
    printf("\n========================================\n");
    printf("Kernel Module Status\n");
    printf("----------------------------------------\n");
    printf("Module Name:    system_monitor\n");

    int loaded = is_module_loaded() || is_module_loaded_via_lsmod();
    printf("Loaded:         %s\n", loaded ? "YES" : "NO");

    struct utsname uts;
    if (uname(&uts) == 0) {
        printf("Kernel Version: %s\n", uts.release);
    } else {
        printf("Kernel Version: Unknown\n");
    }

    char version_str[64] = "1.0";
    if (loaded) {
        FILE *fp = fopen("/proc/sysmgr", "r");
        if (fp) {
            char line[256];
            while (fgets(line, sizeof(line), fp)) {
                if (strncmp(line, "Version:", 8) == 0) {
                    char *val = line + 8;
                    while (*val == ' ' || *val == '\t') val++;
                    size_t len = strlen(val);
                    while (len > 0 && (val[len-1] == '\n' || val[len-1] == '\r' || val[len-1] == ' ' || val[len-1] == '\t')) {
                        val[len-1] = '\0';
                        len--;
                    }
                    strncpy(version_str, val, sizeof(version_str) - 1);
                    version_str[sizeof(version_str) - 1] = '\0';
                    break;
                }
            }
            fclose(fp);
        }
    }
    printf("Module Version: %s\n", version_str);

    int proc_exists = (access("/proc/sysmgr", F_OK) == 0);
    printf("/proc/sysmgr:   %s\n", proc_exists ? "Available" : "Missing");
    printf("========================================\n");
    return 0;
}

static int run_dmesg_filtered(char log_lines[MAX_LOG_LINES][MAX_LINE_LEN], int *log_line_count, int use_sudo, int *exit_code) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        log_error("KERNEL", "pipe creation failed: %s", strerror(errno));
        return -1;
    }
    pid_t pid = fork();
    if (pid == -1) {
        log_error("KERNEL", "fork failed: %s", strerror(errno));
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        if (use_sudo) {
            char *argv[] = {"sudo", "dmesg", NULL};
            execvp("sudo", argv);
        } else {
            char *argv[] = {"dmesg", NULL};
            execvp("dmesg", argv);
        }
        exit(errno);
    }
    close(pipefd[1]);

    FILE *stream = fdopen(pipefd[0], "r");
    if (!stream) {
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        return -1;
    }

    char line_buf[1024];
    *log_line_count = 0;
    int has_permission_error = 0;
    while (fgets(line_buf, sizeof(line_buf), stream)) {
        if (contains_keyword(line_buf, "permission denied") || contains_keyword(line_buf, "not permitted")) {
            has_permission_error = 1;
        }
        if (contains_keyword(line_buf, "system_monitor") || contains_keyword(line_buf, "sysmgr")) {
            line_buf[strcspn(line_buf, "\n")] = '\0';
            strncpy(log_lines[*log_line_count % MAX_LOG_LINES], line_buf, MAX_LINE_LEN - 1);
            log_lines[*log_line_count % MAX_LOG_LINES][MAX_LINE_LEN - 1] = '\0';
            (*log_line_count)++;
        }
    }
    fclose(stream);

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        return -1;
    }
    if (WIFEXITED(status)) {
        *exit_code = WEXITSTATUS(status);
        if (*exit_code != 0 || has_permission_error) {
            return -2;
        }
        return 0;
    }
    return -1;
}

int kernel_mgr_show_log(void) {
    char log_lines[MAX_LOG_LINES][MAX_LINE_LEN];
    int log_line_count = 0;
    int exit_code = 0;

    int res = run_dmesg_filtered(log_lines, &log_line_count, 0, &exit_code);
    if (res == -2 || exit_code != 0) {
        printf("dmesg permission denied. Attempting sudo dmesg...\n");
        res = run_dmesg_filtered(log_lines, &log_line_count, 1, &exit_code);
    }

    if (res != 0 || exit_code != 0) {
        log_error("KERNEL", "Permission denied");
        printf("Error: Permission denied. Cannot access dmesg logs.\n");
        return -1;
    }

    log_info("KERNEL", "Kernel log viewed");
    printf("\n========================================\n");
    printf("Kernel Log (Last 20 Lines)\n");
    printf("----------------------------------------\n");
    if (log_line_count == 0) {
        printf("No logs found matching 'system_monitor' or 'sysmgr'.\n");
    } else {
        int start = 0;
        int limit = log_line_count;
        if (log_line_count > MAX_LOG_LINES) {
            start = log_line_count % MAX_LOG_LINES;
            limit = MAX_LOG_LINES;
        }
        for (int i = 0; i < limit; i++) {
            int idx = (start + i) % MAX_LOG_LINES;
            printf("%s\n", log_lines[idx]);
        }
    }
    printf("========================================\n");
    return 0;
}

void kernel_mgr_run(void) {
    int choice;
    log_info("KERNEL", "Entering Kernel Module Manager");

    while (1) {
        printf("\n========================================\n");
        printf("Kernel Module Manager\n");
        printf("=====================\n");
        printf("1. Show Module Information\n");
        printf("2. Load Kernel Module\n");
        printf("3. Unload Kernel Module\n");
        printf("4. Show Module Status\n");
        printf("5. Show Kernel Log (Last 20 Lines)\n");
        printf("6. Return\n");
        printf("========================================\n");
        printf("Select option: ");
        fflush(stdout);

        choice = read_kernel_choice();

        if (choice < 0) {
            continue;
        }

        if (choice < 1 || choice > 6) {
            printf("\nInvalid input. Please choose a number between 1 and 6.\n");
            kernel_menu_pause();
            continue;
        }

        if (choice == 6) {
            log_info("KERNEL", "Leaving Kernel Module Manager");
            break;
        }

        if (choice == 1) {
            kernel_mgr_show_info();
            kernel_menu_pause();
        } else if (choice == 2) {
            kernel_mgr_load_module();
            kernel_menu_pause();
        } else if (choice == 3) {
            kernel_mgr_unload_module();
            kernel_menu_pause();
        } else if (choice == 4) {
            kernel_mgr_show_status();
            kernel_menu_pause();
        } else if (choice == 5) {
            kernel_mgr_show_log();
            kernel_menu_pause();
        }
    }
}

int kernel_mgr_show_info(void) {
    FILE *fp;
    char buffer[256];
    int line_count = 0;

    log_info("KERNEL", "Kernel module information requested");

    fp = fopen("/proc/sysmgr", "r");
    if (!fp) {
        log_error("KERNEL", "Read failure: failed to open /proc/sysmgr (errno %d)", errno);
        printf("\nKernel module is not loaded.\n");
        printf("Would you like to load it now?\n");
        printf("1. Yes\n");
        printf("2. No\n");
        printf("Select option: ");
        fflush(stdout);
        
        int sub_choice = read_kernel_choice();
        if (sub_choice == 1) {
            kernel_mgr_load_module();
        }
        return -1;
    }

    printf("\n========================================\n");
    printf("Kernel Module Information (/proc/sysmgr)\n");
    printf("----------------------------------------\n");

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
        line_count++;
    }

    printf("========================================\n");

    fclose(fp);

    if (line_count > 0) {
        log_info("KERNEL", "Read success");
        return 0;
    } else {
        log_error("KERNEL", "Read failure: read 0 lines from /proc/sysmgr");
        return -1;
    }
}

