/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/process/demo/process_demo.h
 * Purpose: Declaration of process lifecycle demos.
 */

#ifndef PROCESS_DEMO_H
#define PROCESS_DEMO_H

void fork_demo_run(void);
void exec_demo_run(void);
void wait_demo_run(void);
void zombie_demo_run(void);
void orphan_demo_run(void);
void daemon_demo_run(void);

#endif /* PROCESS_DEMO_H */
