/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: app/socket_client_main.c
 * Purpose: Standalone TCP client binary wrapper.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socket_mgr.h"
#include "logger.h"

int main(int argc, char* argv[]) {
    char ip[128] = "127.0.0.1";
    int port = 8080;
    if (argc > 1) {
        strncpy(ip, argv[1], sizeof(ip) - 1);
        ip[sizeof(ip) - 1] = '\0';
    }
    if (argc > 2) {
        port = atoi(argv[2]);
    }
    log_info("SOCKET", "Socket client launched");
    socket_mgr_client_start(ip, port, "user");
    return 0;
}
