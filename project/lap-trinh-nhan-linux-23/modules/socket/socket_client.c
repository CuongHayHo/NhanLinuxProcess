/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/socket/socket_client.c
 * Purpose: TCP chat client.
 */

#include <stdio.h>
#include <stdlib.h>
#include "socket_mgr.h"
#include "logger.h"

void socket_mgr_client_start(const char* server_ip, int port, const char* username) {
    (void)server_ip;
    (void)port;
    (void)username;
    log_info("SOCKET", "socket_mgr_client_start called (stub).");
    /* TODO: Connect client socket and spawn receiver worker thread */
}
