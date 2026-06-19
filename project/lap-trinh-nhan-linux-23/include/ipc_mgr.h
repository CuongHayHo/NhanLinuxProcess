/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/ipc_mgr.h
 * Purpose: POSIX shared memory IPC manager interfaces.
 */

#ifndef IPC_MGR_H
#define IPC_MGR_H

/**
 * Runs the interactive IPC Manager submenu loop.
 */
void ipc_mgr_run(void);

/**
 * Executes the POSIX Shared Memory demonstration.
 * Returns 0 on success, or -1 on error.
 */
int ipc_mgr_shm_demo(void);

/**
 * Executes the POSIX Named Semaphore demonstration.
 * Returns 0 on success, or -1 on error.
 */
int ipc_mgr_sem_demo(void);

/**
 * Executes the POSIX Producer-Consumer demonstration.
 * Returns 0 on success, or -1 on error.
 */
int ipc_mgr_prod_cons_demo(void);

#endif /* IPC_MGR_H */
