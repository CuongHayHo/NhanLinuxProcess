/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/signal/signal_mgr.c
 * Purpose: System signal handling manager.
 */

#include <stdio.h>
#include <signal.h>
#include "signal_mgr.h"
#include "logger.h"

void signal_mgr_init(void) {
    log_info("SIGNAL", "signal_mgr_init called (stub).");
    /* TODO: Setup signal structures and register with sigaction() */
}

void signal_mgr_restore_defaults(void) {
    log_info("SIGNAL", "signal_mgr_restore_defaults called (stub).");
    /* TODO: Set all registered signal actions to SIG_DFL */
}
