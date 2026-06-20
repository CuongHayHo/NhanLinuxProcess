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
    {"file", "File Manager", "main"},
    {"process", "Process Manager", "main"},
    {"network", "Network Manager", "main"},
    {"socket", "Socket Manager", "main"},
    {"package", "Package Manager", "main"},
    {"shell", "Shell Manager", "main"},
    {"kernel", "Kernel Module", "main"},
    {"help", "Help", "main"},
    {"exit", "Exit", "main"},

    /* File Context */
    {"create", "Create File", "file"},
    {"read", "Read File", "file"},
    {"write", "Write File", "file"},
    {"delete", "Delete File", "file"},
    {"copy", "Copy File", "file"},
    {"move", "Move File", "file"},
    {"rename", "Rename File", "file"},
    {"info", "File Information", "file"},
    {"list", "Directory Listing", "file"},
    {"mkdir", "Create Directory", "file"},
    {"chmod", "Change Permission", "file"},
    {"search", "Search Files", "file"},
    {"archive", "Archive Files", "file"},
    {"back", "Back to Main", "file"},

    /* Process Context */
    {"list", "List Processes", "process"},
    {"info", "Process Information", "process"},
    {"kill", "Kill Process (SIGKILL)", "process"},
    {"signal", "Send custom Signal", "process"},
    {"nice", "Set Priority (Nice)", "process"},
    {"fork", "Parent/Child Demo", "process"},
    {"exec", "Exec Demo", "process"},
    {"wait", "Wait/Waitpid Demo", "process"},
    {"zombie", "Zombie Demo", "process"},
    {"orphan", "Orphan Demo", "process"},
    {"daemon", "Daemon Demo", "process"},
    {"siginit", "Signal Settings", "process"},
    {"sigreset", "Signal Reset", "process"},
    {"back", "Back to Main", "process"},

    /* Network Context */
    {"list", "List Interfaces", "network"},
    {"info", "Interface Information", "network"},
    {"config", "Configure Interface", "network"},
    {"up", "Bring Interface UP", "network"},
    {"down", "Bring Interface DOWN", "network"},
    {"route", "Routing Table", "network"},
    {"ping", "Ping Host", "network"},
    {"dns", "DNS Lookup", "network"},
    {"stats", "Socket Statistics", "network"},
    {"back", "Back to Main", "network"},

    /* Socket Context */
    {"server", "Run TCP Server", "socket"},
    {"client", "Run TCP Client", "socket"},
    {"multi", "Multi Client Echo", "socket"},
    {"chat", "Interactive Chat (Future)", "socket"},
    {"status", "Connection Status", "socket"},
    {"back", "Back to Main", "socket"},

    /* Package Context */
    {"search", "Search Package", "package"},
    {"info", "Package Information", "package"},
    {"install", "Install Package", "package"},
    {"remove", "Remove Package", "package"},
    {"demo", "Safe Demonstration", "package"},
    {"back", "Back to Main", "package"},

    /* Shell Context */
    {"exec", "Execute Command", "shell"},
    {"script", "Run Script", "shell"},
    {"env", "Environment Manager", "shell"},
    {"file", "File Management (shell)", "shell"},
    {"cron", "Task Scheduling (shell)", "shell"},
    {"time", "System Time (shell)", "shell"},
    {"install", "Package Installation (shell)", "shell"},
    {"monitor", "Automation Manager (shell)", "shell"},
    {"back", "Back to Main", "shell"},

    /* Kernel Context */
    {"info", "Module Information", "kernel"},
    {"load", "Load Module", "kernel"},
    {"unload", "Unload Module", "kernel"},
    {"status", "Show Module Status", "kernel"},
    {"log", "Show Kernel Log", "kernel"},
    {"stack", "Network Stack Overview", "kernel"},
    {"skbuff", "sk_buff Overview", "kernel"},
    {"napi", "NAPI Overview", "kernel"},
    {"back", "Back to Main", "kernel"}
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
