/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/network_mgr.h
 * Purpose: Network interface diagnostics.
 */

#ifndef NETWORK_MGR_H
#define NETWORK_MGR_H

#include <netinet/in.h>
#include "common.h"

typedef struct {
    char name[32];
    char ipv4[INET_ADDRSTRLEN];
    char mac[18];
    unsigned long rx_bytes;
    unsigned long tx_bytes;
    int is_up;
} interface_details_t;

void network_mgr_run(void);
int network_mgr_get_interfaces(void);
int network_mgr_get_routing_table(void);
int network_mgr_get_dns_servers(char dns_out[][64], int max_dns);

#endif /* NETWORK_MGR_H */
