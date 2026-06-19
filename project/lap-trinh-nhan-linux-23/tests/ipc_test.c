/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/ipc_test.c
 * Purpose: Complete Diagnostic tests and demonstrations for IPC Shared Memory - Sprint 1.
 */

#include <stdio.h>
#include "ipc_mgr.h"

int main(void) {
    printf("Starting IPC Manager - Sprint 1 (POSIX Shared Memory) test program...\n\n");

    /* Execute Shared Memory demonstration */
    int result = ipc_mgr_shm_demo();
    
    if (result == 0) {
        printf("\nPOSIX Shared Memory IPC test completed successfully.\n");
    } else {
        printf("\nPOSIX Shared Memory IPC test failed.\n");
    }

    return result;
}
