/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/process/demo/daemon_demo.c
 * Purpose: Traditional Linux daemon process creation demonstration.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/wait.h>
#include "process_demo.h"
#include "logger.h"

#define WORKSPACE_PATH "/home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23"

static void daemon_log_info(const char* module, const char* format, ...) {
    char message[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (chdir(WORKSPACE_PATH) == 0) {
        log_info(module, "%s", message);
        if (chdir("/") < 0) {
            /* ignore */
        }
    }
}

static void daemon_log_error(const char* module, const char* format, ...) {
    char message[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (chdir(WORKSPACE_PATH) == 0) {
        log_error(module, "%s", message);
        if (chdir("/") < 0) {
            /* ignore */
        }
    }
}
#define PID_FILE_PATH  WORKSPACE_PATH "/logs/daemon.pid"
#define LOG_FILE_PATH  WORKSPACE_PATH "/logs/daemon_demo.log"

static volatile sig_atomic_t keep_running = 1;

static void handle_sigterm(int sig) {
    (void)sig;
    keep_running = 0;
}

static char get_process_state(pid_t pid) {
    char path[128];
    char buf[1024];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return '\0';
    }
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) {
        return '\0';
    }
    buf[n] = '\0';
    char* p = strstr(buf, "State:");
    if (p) {
        char state = '\0';
        if (sscanf(p, "State: %c", &state) == 1) {
            return state;
        }
    }
    return '\0';
}

void daemon_demo_run(void) {
    log_info("PROCESS", "Daemon demo started");

    printf("\n=== Daemon Process Demonstration ===\n");
    printf("Theory:\n");
    printf("  1. A daemon is a background process that runs detached from any terminal.\n");
    printf("  2. First Fork: The parent exits, making the child orphaned. It runs in the\n");
    printf("     background. setsid() detaches the process from the controlling terminal\n");
    printf("     and creates a new session.\n");
    printf("  3. Second Fork: Ensures the daemon cannot acquire a controlling terminal\n");
    printf("     again (since the second child is not a session leader).\n");
    printf("  4. chdir(\"/\") prevents the daemon from locking a mounted directory.\n");
    printf("  5. umask(0) allows the daemon to read/write files with proper permissions.\n");
    printf("  6. Closing stdin/stdout/stderr detaches it fully from the terminal I/O.\n\n");

    /* Fork a controller process to run the daemonization, so that the grandparent (test harness)
       can remain interactive and wait/terminate the daemon cleanly. */
    pid_t ctrl_pid = fork();

    if (ctrl_pid < 0) {
        log_error("PROCESS", "First fork failed in Daemon demo (errno %d)", errno);
        printf("Error: Fork failed!\n");
        log_info("PROCESS", "Daemon exited");
        return;
    }

    if (ctrl_pid == 0) {
        /* Controller process: performs the actual daemonization sequence */
        
        /* Step 2: First fork */
        pid_t first_child = fork();
        if (first_child < 0) {
            exit(1);
        }
        if (first_child > 0) {
            /* Step 3: Parent exits */
            exit(0);
        }

        /* Step 4: Create new session */
        pid_t sid = setsid();
        if (sid < 0) {
            exit(1);
        }
        daemon_log_info("PROCESS", "Session created");

        /* Step 5: Second fork */
        pid_t daemon_pid = fork();
        if (daemon_pid < 0) {
            exit(1);
        }
        if (daemon_pid > 0) {
            /* Write Daemon PID and Session ID to PID file for grandparent to query */
            int pid_fd = open(PID_FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (pid_fd >= 0) {
                char pid_buf[128];
                snprintf(pid_buf, sizeof(pid_buf), "%d %d", daemon_pid, (int)sid);
                write(pid_fd, pid_buf, strlen(pid_buf));
                close(pid_fd);
            }
            /* Step 6: Parent exits again */
            exit(0);
        }

        /* --- Daemon Process Boundary --- */

        /* Step 7: Set umask */
        umask(0);

        /* Step 8: Change working directory */
        if (chdir("/") < 0) {
            exit(1);
        }

        /* Step 9: Close standard file descriptors */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        /* Redirect descriptors to /dev/null */
        open("/dev/null", O_RDWR); /* stdin */
        open("/dev/null", O_RDWR); /* stdout */
        open("/dev/null", O_RDWR); /* stderr */

        daemon_log_info("PROCESS", "Daemon initialized");

        /* Step 10: Open a log file */
        int log_fd = open(LOG_FILE_PATH, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (log_fd < 0) {
            daemon_log_error("PROCESS", "Daemon failed to open log file (errno %d)", errno);
            exit(1);
        }

        /* Register SIGTERM signal handler */
        if (signal(SIGTERM, handle_sigterm) == SIG_ERR) {
            daemon_log_error("PROCESS", "Daemon failed to register signal handler");
            close(log_fd);
            exit(1);
        }

        /* Step 11: Write timestamped message every few seconds */
        while (keep_running) {
            time_t rawtime;
            struct tm *timeinfo;
            char time_str[64];
            
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            if (timeinfo) {
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
            } else {
                strcpy(time_str, "unknown time");
            }

            char msg[128];
            snprintf(msg, sizeof(msg), "[%s] Daemon heartbeat. PID: %d is running...\n", time_str, getpid());
            write(log_fd, msg, strlen(msg));

            daemon_log_info("PROCESS", "Log written");

            /* Sleep 2 seconds */
            sleep(2);
        }

        /* Step 12: Receive SIGTERM and exit cleanly */
        daemon_log_info("PROCESS", "SIGTERM received");
        
        char term_msg[] = "Daemon received SIGTERM. Exiting cleanly.\n";
        write(log_fd, term_msg, strlen(term_msg));
        close(log_fd);

        daemon_log_info("PROCESS", "Daemon exited");
        exit(0);
    }

    /* Grandparent process: wait for controller to exit, indicating daemon is detached */
    int status;
    waitpid(ctrl_pid, &status, 0);

    /* Read the Daemon's PID and SID from the pid file */
    pid_t d_pid = 0;
    pid_t d_sid = 0;
    
    /* Allow daemon a brief moment to write the PID file */
    sleep(1);
    
    int pid_fd = open(PID_FILE_PATH, O_RDONLY);
    if (pid_fd >= 0) {
        char pid_buf[128];
        ssize_t n = read(pid_fd, pid_buf, sizeof(pid_buf) - 1);
        close(pid_fd);
        if (n > 0) {
            pid_buf[n] = '\0';
            sscanf(pid_buf, "%d %d", &d_pid, &d_sid);
        }
    }

    if (d_pid <= 0) {
        printf("Error: Failed to retrieve daemon PID.\n");
        log_info("PROCESS", "Demo finished");
        return;
    }

    printf("Daemon PID:        %d\n", d_pid);
    printf("Session ID:        %d\n", d_sid);
    printf("Working Directory: /\n");
    printf("Log File Path:     %s\n", LOG_FILE_PATH);
    printf("Current Status:    Running (check logs)\n\n");

    printf("[Grandparent] Waiting 3 seconds for daemon to write heartbeats...\n");
    sleep(3);

    /* Display log output from the daemon log */
    printf("\n--- Daemon Log Output ---\n");
    int log_fd = open(LOG_FILE_PATH, O_RDONLY);
    if (log_fd >= 0) {
        char log_buf[2048];
        ssize_t n = read(log_fd, log_buf, sizeof(log_buf) - 1);
        close(log_fd);
        if (n > 0) {
            log_buf[n] = '\0';
            printf("%s", log_buf);
        }
    } else {
        printf("Error: Failed to read daemon log file.\n");
    }
    printf("-------------------------\n\n");

    /* Terminate the daemon */
    printf("[Grandparent] Sending SIGTERM to Daemon PID %d...\n", d_pid);
    if (kill(d_pid, SIGTERM) < 0) {
        printf("Error: Failed to send SIGTERM to daemon.\n");
    } else {
        printf("Current Status:    SIGTERM sent\n");
    }

    /* Wait a moment for daemon to exit and write cleanup log */
    sleep(2);

    printf("\n--- Daemon Log Output (Final Check) ---\n");
    log_fd = open(LOG_FILE_PATH, O_RDONLY);
    if (log_fd >= 0) {
        char log_buf[2048];
        ssize_t n = read(log_fd, log_buf, sizeof(log_buf) - 1);
        close(log_fd);
        if (n > 0) {
            log_buf[n] = '\0';
            printf("%s", log_buf);
        }
    }
    printf("-------------------------\n");

    /* Verify if the daemon process is gone or has transitioned to zombie state (clean termination under container environment) */
    int exited_cleanly = 0;
    char proc_path[128];
    snprintf(proc_path, sizeof(proc_path), "/proc/%d", d_pid);
    int proc_fd = open(proc_path, O_RDONLY);
    if (proc_fd < 0) {
        exited_cleanly = 1;
    } else {
        close(proc_fd);
        char state = get_process_state(d_pid);
        if (state == 'Z') {
            exited_cleanly = 1;
        }
    }

    if (exited_cleanly) {
        printf("Termination Status: Exited cleanly\n");
    } else {
        printf("Termination Status: Warning: Daemon PID still active!\n");
    }

    /* Cleanup temporary files */
    unlink(PID_FILE_PATH);

    log_info("PROCESS", "Demo finished");
    printf("====================================\n");
}
