/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/kernel_test.c
 * Purpose: Complete Diagnostic tests and demonstrations for Kernel Manager - Sprint 3.
 */

#include <stdio.h>
#include "kernel_mgr.h"

int main(void) {
    printf("Starting Kernel Module Manager - Sprint 3 (User Space Integration) test program...\n\n");

    /* Call API to inspect module details from /proc/sysmgr */
    int result = kernel_mgr_show_info();
    
    if (result == 0) {
        printf("\nKernel module information read successfully (Module is LOADED).\n");
    } else {
        printf("\nKernel module information read failed (Module is NOT LOADED, or /proc/sysmgr is unavailable).\n");
    }

    printf("\nKernel Module Manager tests completed.\n");
    return 0;
}
