/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/network_test.c
 * Purpose: Complete Diagnostic tests and demonstrations for Network Manager Sprint 3.
 */

#include <stdio.h>
#include "network_mgr.h"

int main(void) {
    printf("Starting Network Manager Module - Sprint 3 (Host Information) test program...\n\n");
    
    printf("--- Test 1: List Network Interfaces ---\n");
    int iface_res = network_list_interfaces();
    
    printf("\n--- Test 2: Display Active Routing Information & Default Gateway ---\n");
    int route_res = network_show_routes();
    
    printf("\n--- Test 3: Routing Parser Verification with Mock Data ---\n");
    const char* mock_route_table =
        "Iface\tDestination\tGateway \tFlags\tRefCnt\tUse\tMetric\tMask\t\tMTU\tWindow\tIRTT\n"
        "eth0\t00020A0A\t00000000\t0001\t0\t0\t100\t00FFFFFF\t0\t0\t0\n"
        "eth0\t00000000\t01020A0A\t0003\t0\t0\t100\t00000000\t0\t0\t0\n";
    int mock_res = parse_routing_table(mock_route_table);
    
    printf("\n--- Test 4: Host Information ---\n");
    int host_res = network_show_host_info();
    
    if (iface_res == 0 && route_res == 0 && mock_res == 0 && host_res == 0) {
        printf("\nNetwork Manager Module tests completed successfully.\n");
        return 0;
    } else {
        printf("\nNetwork Manager Module tests failed.\n");
        return 1;
    }
}
