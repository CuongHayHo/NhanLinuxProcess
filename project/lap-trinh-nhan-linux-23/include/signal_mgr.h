/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/signal_mgr.h
 * Purpose: Signal capture and action handlers.
 */

#ifndef SIGNAL_MGR_H
#define SIGNAL_MGR_H

void signal_mgr_init(void);
void signal_mgr_restore_defaults(void);

#endif /* SIGNAL_MGR_H */
