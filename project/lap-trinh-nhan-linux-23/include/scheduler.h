/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/scheduler.h
 * Purpose: Scheduled tasks crontab manager.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "common.h"

void scheduler_run(void);
int scheduler_list_jobs(void);
int scheduler_add_job(const char* cron_expr, const char* script_path);
int scheduler_remove_jobs(void);

#endif /* SCHEDULER_H */
