/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/logger_test.c
 * Purpose: Diagnostic tests and demonstrations for the Logger module.
 */

#include <stdio.h>
#include "logger.h"

int main(void) {
    printf("Starting Logger test program...\n");
    
    /* Log messages of different severities and source modules */
    log_info("SYSTEM", "Application started");
    log_warning("FILE", "Permission denied");
    log_error("NETWORK", "Connection timeout");
    
    printf("Logger test complete. Please inspect 'logs/system.log' to verify results.\n");
    return 0;
}
