/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: common/terminal_launcher.c
 * Purpose: Terminal detection and external window execution helper.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "terminal_launcher.h"
#include "logger.h"

static const char* terminals[] = {
    "ptyxis",
    "gnome-terminal",
    "konsole",
    "kitty",
    "wezterm",
    "xfce4-terminal",
    "xterm"
};
#define TERM_COUNT (sizeof(terminals) / sizeof(terminals[0]))

static int command_exists(const char* cmd) {
    char* path = getenv("PATH");
    if (!path) return 0;
    
    char* path_copy = strdup(path);
    char* dir = strtok(path_copy, ":");
    while (dir != NULL) {
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);
        struct stat st;
        if (stat(full_path, &st) == 0 && (st.st_mode & S_IXUSR)) {
            free(path_copy);
            return 1;
        }
        dir = strtok(NULL, ":");
    }
    free(path_copy);
    return 0;
}

const char* detect_terminal(void) {
    for (size_t i = 0; i < TERM_COUNT; i++) {
        if (command_exists(terminals[i])) {
            return terminals[i];
        }
    }
    return NULL;
}

int terminal_open(const char* title, const char* command) {
    char* tmux_env = getenv("TMUX");
    if (tmux_env && command_exists("tmux")) {
        log_info("SOCKET", "TMUX session detected. Spawning split-window...");
        pid_t pid = fork();
        if (pid < 0) {
            log_error("LAUNCHER", "Fork failed");
            return -1;
        }
        if (pid == 0) {
            int dev_null = open("/dev/null", O_WRONLY);
            if (dev_null >= 0) {
                dup2(dev_null, 1);
                dup2(dev_null, 2);
                close(dev_null);
            }
            char full_cmd[1024];
            snprintf(full_cmd, sizeof(full_cmd), "%s; echo; read -p 'Press Enter to close...'", command);
            
            char* exec_argv[] = {
                "tmux",
                "split-window",
                "-h",
                "sh",
                "-c",
                full_cmd,
                NULL
            };
            execvp("tmux", exec_argv);
            perror("execvp tmux failed");
            exit(1);
        }
        return 0;
    }

    const char* term = detect_terminal();
    if (!term) {
        log_error("LAUNCHER", "No supported terminal found! (Pro tip: run inside a tmux session to support headless terminal splits)");
        return -1;
    }
    
    // Log Terminal launched
    log_info("SOCKET", "Terminal launched");
    
    pid_t pid = fork();
    if (pid < 0) {
        log_error("LAUNCHER", "Fork failed");
        return -1;
    }
    
    if (pid == 0) {
        // Mute GUI debug logs / GTK warnings by redirecting output to /dev/null
        int dev_null = open("/dev/null", O_WRONLY);
        if (dev_null >= 0) {
            dup2(dev_null, 1);
            dup2(dev_null, 2);
            close(dev_null);
        }

        char full_cmd[1024];
        snprintf(full_cmd, sizeof(full_cmd), "%s; echo; read -p 'Press Enter to close...'", command);
        
        char* exec_argv[16];
        int idx = 0;
        
        exec_argv[idx++] = (char*)term;
        
        if (strcmp(term, "ptyxis") == 0) {
            exec_argv[idx++] = "--title";
            exec_argv[idx++] = (char*)title;
            exec_argv[idx++] = "--";
            exec_argv[idx++] = "sh";
            exec_argv[idx++] = "-c";
            exec_argv[idx++] = full_cmd;
        } else if (strcmp(term, "gnome-terminal") == 0) {
            exec_argv[idx++] = "--title";
            exec_argv[idx++] = (char*)title;
            exec_argv[idx++] = "--";
            exec_argv[idx++] = "sh";
            exec_argv[idx++] = "-c";
            exec_argv[idx++] = full_cmd;
        } else if (strcmp(term, "konsole") == 0) {
            exec_argv[idx++] = "--title";
            exec_argv[idx++] = (char*)title;
            exec_argv[idx++] = "-e";
            exec_argv[idx++] = "sh";
            exec_argv[idx++] = "-c";
            exec_argv[idx++] = full_cmd;
        } else if (strcmp(term, "kitty") == 0) {
            exec_argv[idx++] = "--title";
            exec_argv[idx++] = (char*)title;
            exec_argv[idx++] = "sh";
            exec_argv[idx++] = "-c";
            exec_argv[idx++] = full_cmd;
        } else if (strcmp(term, "wezterm") == 0) {
            exec_argv[idx++] = "start";
            exec_argv[idx++] = "--title";
            exec_argv[idx++] = (char*)title;
            exec_argv[idx++] = "--";
            exec_argv[idx++] = "sh";
            exec_argv[idx++] = "-c";
            exec_argv[idx++] = full_cmd;
        } else if (strcmp(term, "xfce4-terminal") == 0) {
            exec_argv[idx++] = "--title";
            exec_argv[idx++] = (char*)title;
            exec_argv[idx++] = "-e";
            exec_argv[idx++] = full_cmd;
        } else if (strcmp(term, "xterm") == 0) {
            exec_argv[idx++] = "-T";
            exec_argv[idx++] = (char*)title;
            exec_argv[idx++] = "-hold";
            exec_argv[idx++] = "-e";
            exec_argv[idx++] = "sh";
            exec_argv[idx++] = "-c";
            exec_argv[idx++] = full_cmd;
        }
        
        exec_argv[idx] = NULL;
        
        execvp(term, exec_argv);
        perror("execvp terminal failed");
        exit(1);
    }
    
    return 0;
}
