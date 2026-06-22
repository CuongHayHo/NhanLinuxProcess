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
        fprintf(stderr, "\n\033[1;31m[Error]\033[0m No supported desktop GUI terminal emulator found on this system.\n");
        fprintf(stderr, "\033[1;36m[Pro Tip]\033[0m Since you are running in a headless SSH environment, please install and run 'tmux' first:\n");
        fprintf(stderr, "          1. Run: \033[1;32msudo apt install tmux -y && tmux\033[0m\n");
        fprintf(stderr, "          2. Run: \033[1;32m./sysmgr\033[0m\n");
        fprintf(stderr, "          This will enable automatic horizontal terminal splits for socket tools.\n\n");
        fflush(stderr);
        log_error("LAUNCHER", "No supported terminal found!");
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

void print_ssh_tunnel_guide(int port) {
    char* ssh_connection = getenv("SSH_CONNECTION");
    if (!ssh_connection) {
        ssh_connection = getenv("SSH_CLIENT");
    }

    char server_ip[64] = "<SSH_IP>";
    char ssh_port[16] = "<SSH_PORT>";
    
    if (ssh_connection) {
        char client_ip[64], client_port[16], s_ip[64], s_port[16];
        if (sscanf(ssh_connection, "%s %s %s %s", client_ip, client_port, s_ip, s_port) >= 3) {
            strncpy(server_ip, s_ip, sizeof(server_ip) - 1);
            server_ip[sizeof(server_ip) - 1] = '\0';
        }
    }
    
    printf("\n\033[1;36m======================================================================\033[0m\n");
    printf("\033[1;33m       SSH PORT FORWARDING / TUNNELING GUIDE (HEADLESS DOCKER)\033[0m\n");
    printf("======================================================================\n");
    printf("If you are running this inside a headless SSH Docker container, your\n");
    printf("local machine cannot connect directly. Use one of these methods:\n\n");
    printf("\033[1;32mMETHOD A: Run Server on Docker, connect from Local PC (Client)\033[0m\n");
    printf("  1. On your local PC, start SSH with Local Port Forwarding:\n");
    printf("     \033[1;37mssh -p %s -L %d:localhost:%d root@%s\033[0m\n", ssh_port, port, port, server_ip);
    printf("  2. In this Docker terminal, start the server on port %d.\n", port);
    printf("  3. On your local PC, start client connecting to \033[1;37m127.0.0.1 %d\033[0m.\n\n", port);
    printf("\033[1;32mMETHOD B: Run Server on Local PC, connect from Docker (Client)\033[0m\n");
    printf("  1. On your local PC, start the server on port %d.\n", port);
    printf("  2. On your local PC, start SSH with Remote Port Forwarding:\n");
    printf("     \033[1;37mssh -p %s -R %d:localhost:%d root@%s\033[0m\n", ssh_port, port, port, server_ip);
    printf("  3. In this Docker terminal, start client connecting to \033[1;37m127.0.0.1 %d\033[0m.\n", port);
    printf("======================================================================\n\n");
    fflush(stdout);
}
