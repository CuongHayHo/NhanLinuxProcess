/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/producer_consumer_test.c
 * Purpose: Complete Diagnostic tests and demonstrations for POSIX Producer Consumer - IPC Sprint 3.
 */

#include <stdio.h>
#include "ipc_mgr.h"

int main(void) {
    printf("Starting IPC Manager - Sprint 3 (POSIX Producer Consumer Integration) test program...\n\n");

    /* Execute POSIX Producer-Consumer integration demonstration */
    int result = ipc_mgr_prod_cons_demo();
    
    if (result == 0) {
        printf("\nPOSIX Producer Consumer Integration test completed successfully.\n");
    } else {
        printf("\nPOSIX Producer Consumer Integration test failed.\n");
    }

    return result;
}
