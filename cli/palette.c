/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: cli/palette.c
 * Purpose: Command Palette database and search operations.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "palette.h"
#include "ui.h"

const palette_item_t palette_items[] = {
    /* Main Context */
    {"file", "Enter File Manager (CRUD, permissions, directory listing, search, zip/archive)", "main"},
    {"process", "Enter Process Manager (list active processes, kill/send signal, nice priority, demos)", "main"},
    {"network", "Enter Network Manager (list interfaces, configure IP, up/down link, route, ping, DNS)", "main"},
    {"socket", "Enter Socket Manager (start TCP single/multi servers, client connection, chat interactive)", "main"},
    {"package", "Enter Package Manager (search, inspect, install/remove packages, dry-run sandbox)", "main"},
    {"shell", "Enter Shell Manager (run custom commands, pre-loaded shell scripts, edit env variables)", "main"},
    {"cron", "Task Scheduling / Cron Job Manager (list, create, delete scheduled tasks)", "main"},
    {"time", "System Time and NTP Synchronization Manager (show, set, sync)", "main"},
    {"kernel", "Enter Kernel Module (view/load/unload sys_monitor module, network stack overview)", "main"},
    {"help", "Show commands help cheat sheet and description lists", "main"},
    {"exit", "Safely terminate and exit the Linux System Manager program", "main"},

    /* File Context */
    {"create", "Create a new blank file at a specified workspace path", "file"},
    {"read", "Read and output the text content of a file to the terminal", "file"},
    {"write", "Write or overwrite new text content into a specific file", "file"},
    {"delete", "Delete a file or directory permanently from the system", "file"},
    {"copy", "Copy a file or directory to a different target location", "file"},
    {"move", "Move a file or directory to another target directory path", "file"},
    {"rename", "Rename an existing file or directory in place", "file"},
    {"info", "Show detailed metadata information of a file (size, path, type)", "file"},
    {"list", "List all files and subdirectories in a directory path", "file"},
    {"mkdir", "Create a new directory at a specified target path", "file"},
    {"chmod", "Change file or directory read/write/execute permissions (octal mode)", "file"},
    {"search", "Search for files by name matching a search query", "file"},
    {"archive", "Compress files or directories into a tar.gz archive", "file"},
    {"back", "Go back to the parent 'main' system context prompt", "file"},

    /* Process Context */
    {"list", "List all running system processes with PID, PPID, and state", "process"},
    {"info", "Get detailed metrics for a specific process by name or PID", "process"},
    {"kill", "Forcefully terminate a process using the SIGKILL (9) signal", "process"},
    {"signal", "Send a custom signal number (e.g. 15 for SIGTERM) to a process", "process"},
    {"nice", "Change the nice CPU scheduling priority value of a process (-20 to 19)", "process"},
    {"fork", "Demonstrate parent/child process creation using fork() system call", "process"},
    {"exec", "Demonstrate how a process memory image is replaced using exec()", "process"},
    {"wait", "Demonstrate how parent waits for child termination using wait()/waitpid()", "process"},
    {"zombie", "Spawn and inspect a temporary zombie process demonstration", "process"},
    {"orphan", "Spawn and inspect a temporary orphan process demonstration", "process"},
    {"daemon", "Demonstrate background system daemon execution flow", "process"},
    {"siginit", "Initialize custom handler for SIGINT/SIGTERM terminal signals", "process"},
    {"sigreset", "Restore standard system handlers for terminal signals", "process"},
    {"back", "Go back to the parent 'main' system context prompt", "process"},

    /* Network Context */
    {"list", "List all available network interfaces on this machine", "network"},
    {"info", "Show details (IP address, MAC, status) of a specific interface", "network"},
    {"config", "Configure IP address and subnet mask of a network interface", "network"},
    {"up", "Bring a network interface link status UP (active)", "network"},
    {"down", "Bring a network interface link status DOWN (inactive)", "network"},
    {"route", "Show the current kernel routing table configuration", "network"},
    {"ping", "Send ICMP Echo requests to verify connection to a remote host", "network"},
    {"dns", "Query DNS name servers to resolve domain names to IP addresses", "network"},
    {"stats", "Show network socket statistics using the ss tool", "network"},
    {"back", "Go back to the parent 'main' system context prompt", "network"},

    /* Socket Context */
    {"server", "Launch a single-connection interactive TCP Chat Server (termios raw mode)", "socket"},
    {"client", "Launch an interactive TCP Client to connect to a remote TCP Server", "socket"},
    {"multi", "Launch a multi-threaded concurrent TCP Echo Server in the background", "socket"},
    {"chat host", "Launch a TCP Chat Host Server (termios raw mode)", "socket"},
    {"chat client", "Launch a TCP Chat Client to connect to a Chat Host", "socket"},
    {"chat demo", "Launch an automated TCP Chat Demo (Host + Client windows)", "socket"},
    {"status", "View active system network connections using the ss socket utility", "socket"},
    {"back", "Go back to the parent 'main' system context prompt", "socket"},

    /* Package Context */
    {"search", "Search packages in system repositories (e.g. apt, dnf, rpm)", "package"},
    {"info", "Show metadata and descriptions of a specific package", "package"},
    {"install", "Install a package from system repositories to this system", "package"},
    {"remove", "Uninstall/remove a package from this system", "package"},
    {"demo", "Simulate package manager actions safely in dry-run sandbox", "package"},
    {"setup", "Install all essential packages (e.g. ping, curl, tmux, headers) for sysmgr modules", "package"},
    {"back", "Go back to the parent 'main' system context prompt", "package"},

    /* Shell Context */
    {"exec", "Execute any standard shell command directly in /bin/sh", "shell"},
    {"script backup.sh", "Execute pre-loaded backup shell script", "shell"},
    {"script disk_usage.sh", "Execute pre-loaded disk usage statistics shell script", "shell"},
    {"script show_date.sh", "Execute pre-loaded show system date shell script", "shell"},
    {"env", "Environment Manager", "shell"},
    {"file", "File Management (shell)", "shell"},
    {"cron", "Task Scheduling (shell)", "shell"},
    {"time", "System Time (shell)", "shell"},
    {"install", "Package Installation (shell)", "shell"},
    {"monitor", "Automation Manager (shell)", "shell"},
    {"back", "Back to Main", "shell"},

    /* Kernel Context */
    {"info", "Show metadata of the custom sys_monitor kernel module", "kernel"},
    {"load", "Compile and load (insmod) sys_monitor kernel module", "kernel"},
    {"unload", "Unload (rmmod) the sys_monitor kernel module from the kernel", "kernel"},
    {"status", "Check if the sys_monitor module is loaded in the active kernel", "kernel"},
    {"log", "Dump kernel log messages (dmesg) related to system_monitor", "kernel"},
    {"stack", "Show overview of the Linux kernel network stack layers", "kernel"},
    {"skbuff", "Show overview of the Linux kernel sk_buff packet buffer struct", "kernel"},
    {"napi", "Show overview of the Linux kernel NAPI high-performance polling design", "kernel"},
    {"back", "Go back to the parent 'main' system context prompt", "kernel"},

    /* Cron Context subcommands */
    {"list", "List all scheduled cron tasks", "cron"},
    {"create", "Create a new scheduled cron task", "cron"},
    {"delete", "Delete all scheduled cron tasks", "cron"},

    /* Time Context subcommands */
    {"show", "Show current system time and date", "time"},
    {"zone", "Show active system timezone", "time"},
    {"set", "Set system time manually (requires root)", "time"},
    {"sync", "Synchronize time with NTP server immediately", "time"},
    {"autosync", "Enable automatic network time synchronization (NTP)", "time"}
};

const int palette_items_count = sizeof(palette_items) / sizeof(palette_items[0]);

/* Helper to convert string to lowercase */
static void to_lowercase(const char* src, char* dest, size_t max_len) {
    size_t i = 0;
    while (src[i] && i < max_len - 1) {
        dest[i] = tolower((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

void palette_search_and_show(const char* current_context, const char* search_query) {
    char clean_query[128] = "";
    
    /* Remove leading '/' if present */
    if (search_query && search_query[0] == '/') {
        to_lowercase(search_query + 1, clean_query, sizeof(clean_query));
    } else if (search_query) {
        to_lowercase(search_query, clean_query, sizeof(clean_query));
    }

    printf("\n%s================= Command Palette =================%s\n", ANSI_CYAN, ANSI_RESET);
    ui_print_info("Context: %s\n", current_context);
    if (strlen(clean_query) > 0) {
        ui_print_warning("Filter:  \"%s\"\n", clean_query);
    }
    printf("--------------------------------------------------\n");
    ui_print_bold("  %-16s  %s\n", "Command", "Description");
    printf("  %-16s  %s\n", "-------", "-----------");

    int matches = 0;
    for (int i = 0; i < palette_items_count; i++) {
        const palette_item_t* item = &palette_items[i];
        
        /* Filter by current context namespace */
        if (strcmp(item->context, current_context) != 0) {
            continue;
        }

        /* Check search filter */
        if (strlen(clean_query) > 0) {
            char lower_cmd[128];
            char lower_disp[128];
            to_lowercase(item->command, lower_cmd, sizeof(lower_cmd));
            to_lowercase(item->display, lower_disp, sizeof(lower_disp));
            
            if (strstr(lower_cmd, clean_query) == NULL && strstr(lower_disp, clean_query) == NULL) {
                continue;
            }
        }

        printf("  /%-15s  %s\n", item->command, item->display);
        matches++;
    }

    if (matches == 0) {
        ui_print_error("  No matching commands found.\n");
    }
    printf("==================================================\n\n");
    fflush(stdout);
}
