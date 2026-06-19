/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/network/network_mgr.c
 * Purpose: Network interface and gateway routing metrics collector.
 */

#include <stdio.h>
#include <stdlib.h>
#include "network_mgr.h"
#include "logger.h"

void network_mgr_run(void) {
    printf("--- Network Manager (Stub) ---\n");
    printf("1. Print interfaces stub\n");
    printf("2. Print gateway/routing table stub\n");
    printf("0. Return\n");
    log_info("NETWORK", "Network manager panel invoked.");
    
    /* TODO: Call helper functions to parse interfaces */
}

int network_mgr_get_interfaces(void) {
    log_info("NETWORK", "network_mgr_get_interfaces called (stub).");
    /* TODO: Fetch active network adapters using getifaddrs() */
    return 0;
}

int network_mgr_get_routing_table(void) {
    log_info("NETWORK", "network_mgr_get_routing_table called (stub).");
    /* TODO: Parse /proc/net/route for gateway settings */
    return 0;
}

int network_mgr_get_dns_servers(char dns_out[][64], int max_dns) {
    (void)dns_out;
    (void)max_dns;
    log_info("NETWORK", "network_mgr_get_dns_servers called (stub).");
    /* TODO: Parse DNS addresses from /etc/resolv.conf */
    return 0;
}
