/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/scheduler.h
 * Purpose: Task scheduler in-memory data structures and API declarations.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "common.h"

#define MAX_TASK_NAME 64
#define MAX_TASK_CMD 128
#define MAX_TIME_STR 32
#define MAX_TASKS 64

typedef struct {
    int id;
    char name[MAX_TASK_NAME];
    char command[MAX_TASK_CMD];
    char scheduled_time[MAX_TIME_STR];
    char status[16];
    char creation_time[MAX_TIME_STR];
} scheduler_task_t;

/**
 * Runs the interactive Task Scheduler submenu.
 */
void scheduler_run(void);

/**
 * Adds a new task to the scheduler memory database.
 * Returns task ID on success, or -1 on error.
 */
int scheduler_task_create(const char* name, const char* command, const char* sched_time);

/**
 * Lists all tasks currently in the scheduler database.
 * Returns the number of tasks, or -1 on error.
 */
int scheduler_task_list(void);

/**
 * Deletes a task by ID.
 * Returns 0 on success, or -1 on error.
 */
int scheduler_task_delete(int task_id);

#endif /* SCHEDULER_H */
