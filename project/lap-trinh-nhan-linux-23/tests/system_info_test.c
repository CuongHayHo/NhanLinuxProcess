/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/system_info_test.c
 * Purpose: Diagnostic tests and demonstrations for System Information module.
 */

#include <stdio.h>
#include "system_info.h"

int main(void) {
    printf("Starting System Information Module - Sprint 1 test program...\n\n");
    system_info_run();
    printf("\nSystem Information Module tests completed successfully.\n");
    return 0;
}
