/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: app/socket_chat_main.c
 * Purpose: Standalone TCP chat application supporting host and client modes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socket_mgr.h"
#include "logger.h"

int main(int argc, char* argv[]) {
    int port = 8080;
    char ip[128] = "127.0.0.1";
    int is_host = 0;
    int is_client = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--host") == 0) {
            is_host = 1;
            if (i + 1 < argc) {
                port = atoi(argv[i + 1]);
            }
        } else if (strcmp(argv[i], "--client") == 0) {
            is_client = 1;
            if (i + 1 < argc) {
                strncpy(ip, argv[i + 1], sizeof(ip) - 1);
                ip[sizeof(ip) - 1] = '\0';
            }
            if (i + 2 < argc) {
                port = atoi(argv[i + 2]);
            }
        }
    }

    if (is_host) {
        log_info("SOCKET", "Chat host launched");
        printf("Starting Chat Host on port %d...\n", port);
        socket_mgr_server_start(port);
    } else if (is_client) {
        log_info("SOCKET", "Chat client launched");
        printf("Connecting to Chat Host at %s:%d...\n", ip, port);
        socket_mgr_client_start(ip, port, "client_user");
    } else {
        printf("Usage:\n");
        printf("  socket_chat --host [port]\n");
        printf("  socket_chat --client [ip] [port]\n");
        return 1;
    }

    return 0;
}
