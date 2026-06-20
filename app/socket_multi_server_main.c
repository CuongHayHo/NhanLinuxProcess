/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: app/socket_multi_server_main.c
 * Purpose: Standalone multi-client TCP echo server binary wrapper.
 */

#include <stdio.h>
#include <stdlib.h>
#include "socket_mgr.h"
#include "logger.h"

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    log_info("SOCKET", "Socket server launched");
    socket_mgr_multi_server_start(port);
    return 0;
}
