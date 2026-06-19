/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/socket_mgr.h
 * Purpose: Chat client-server network socket interfaces.
 */

#ifndef SOCKET_MGR_H
#define SOCKET_MGR_H

#include "common.h"

void socket_mgr_server_start(int port);
void socket_mgr_client_start(const char* server_ip, int port, const char* username);

#endif /* SOCKET_MGR_H */
