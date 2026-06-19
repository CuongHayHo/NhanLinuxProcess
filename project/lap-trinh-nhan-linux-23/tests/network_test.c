/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/network_test.c
 * Purpose: Complete Diagnostic tests and demonstrations for Network Manager module.
 */

#include <stdio.h>
#include "network_mgr.h"

int main(void) {
    printf("Starting Network Manager Module - Sprint 2 test program...\n\n");
    
    printf("--- Test 1: Display Hostname ---\n");
    network_show_hostname();
    
    printf("\n--- Test 2: List IP Addresses ---\n");
    network_show_ip();
    
    printf("\n--- Test 3: List MAC Addresses ---\n");
    network_show_mac();

    printf("\n--- Test 4: Default Gateway ---\n");
    network_show_gateway();

    printf("\n--- Test 5: DNS Servers ---\n");
    network_show_dns();

    printf("\n--- Test 6: Routing Table ---\n");
    network_show_routes();

    printf("\n--- Test 7: Network Statistics ---\n");
    network_show_statistics();
    
    printf("\n--- Test 8: Consolidation Panel Run ---\n");
    network_mgr_run();
    
    printf("\nNetwork Manager Module tests completed successfully.\n");
    return 0;
}
