/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/scheduler/scheduler.c
 * Purpose: In-memory task scheduler database and submenu loops.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "scheduler.h"
#include "logger.h"

static scheduler_task_t tasks[MAX_TASKS];
static int task_count = 0;
static int next_id = 1;

static int read_scheduler_choice(void) {
    char input_buf[128];
    char* endptr;
    long val;

    if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
        return -1;
    }

    input_buf[strcspn(input_buf, "\n")] = '\0';
    if (strlen(input_buf) == 0) {
        return -1;
    }

    val = strtol(input_buf, &endptr, 10);
    if (*endptr != '\0') {
        return -1;
    }
    return (int)val;
}

static void scheduler_menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF);
}

static void get_current_time_str(char* buf, size_t size) {
    time_t t = time(NULL);
    struct tm tm_local;
    if (t != (time_t)-1 && localtime_r(&t, &tm_local) != NULL) {
        strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm_local);
    } else {
        strncpy(buf, "N/A", size - 1);
        buf[size - 1] = '\0';
    }
}

int scheduler_task_create(const char* name, const char* command, const char* sched_time) {
    if (name == NULL || strlen(name) == 0 || command == NULL || strlen(command) == 0 || sched_time == NULL || strlen(sched_time) == 0) {
        log_error("SCHEDULER", "Task creation failed: invalid arguments");
        printf("\nError: Task parameters cannot be empty.\n");
        return -1;
    }

    if (task_count >= MAX_TASKS) {
        log_error("SCHEDULER", "Task creation failed: database limit reached (%d)", MAX_TASKS);
        printf("\nError: Scheduler task limit reached (%d).\n", MAX_TASKS);
        return -1;
    }

    scheduler_task_t* t = &tasks[task_count];
    t->id = next_id++;
    strncpy(t->name, name, sizeof(t->name) - 1);
    t->name[sizeof(t->name) - 1] = '\0';
    strncpy(t->command, command, sizeof(t->command) - 1);
    t->command[sizeof(t->command) - 1] = '\0';
    strncpy(t->scheduled_time, sched_time, sizeof(t->scheduled_time) - 1);
    t->scheduled_time[sizeof(t->scheduled_time) - 1] = '\0';
    strcpy(t->status, "PENDING");
    
    get_current_time_str(t->creation_time, sizeof(t->creation_time));

    task_count++;
    log_info("SCHEDULER", "Task created: ID=%d, Name=%s", t->id, t->name);
    return t->id;
}

int scheduler_task_list(void) {
    log_info("SCHEDULER", "Task listed (Count: %d)", task_count);

    if (task_count == 0) {
        printf("\nNo tasks scheduled.\n");
        return 0;
    }

    printf("\n========================================================================================================\n");
    printf("%-5s %-20s %-25s %-20s %-12s %-20s\n", "ID", "Task Name", "Command", "Scheduled Time", "Status", "Creation Time");
    printf("--------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < task_count; i++) {
        scheduler_task_t* t = &tasks[i];
        printf("%-5d %-20s %-25s %-20s %-12s %-20s\n", 
               t->id, t->name, t->command, t->scheduled_time, t->status, t->creation_time);
    }
    printf("========================================================================================================\n");

    return task_count;
}

int scheduler_task_delete(int task_id) {
    int found_idx = -1;
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].id == task_id) {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1) {
        log_error("SCHEDULER", "Task deletion failed: ID %d not found", task_id);
        printf("\nError: Task ID %d not found.\n", task_id);
        return -1;
    }

    log_info("SCHEDULER", "Task removed: ID=%d", task_id);

    // Shift left to fill gap
    for (int i = found_idx; i < task_count - 1; i++) {
        tasks[i] = tasks[i + 1];
    }
    task_count--;
    printf("\nTask ID %d deleted successfully.\n", task_id);
    return 0;
}

void scheduler_run(void) {
    int choice;
    log_info("SCHEDULER", "Scheduler menu invoked.");

    while (1) {
        printf("\n========================================\n");
        printf("               Scheduler\n");
        printf("========================================\n");
        printf("1. Create Task\n");
        printf("2. List Tasks\n");
        printf("3. Delete Task\n");
        printf("4. Execute Scheduler (Future)\n");
        printf("0. Return\n");
        printf("========================================\n");
        printf("Select option: ");
        fflush(stdout);

        choice = read_scheduler_choice();

        if (choice < 0) {
            continue;
        }

        if (choice > 4) {
            printf("\nInvalid input. Please choose a number between 0 and 4.\n");
            scheduler_menu_pause();
            continue;
        }

        if (choice == 0) {
            log_info("SCHEDULER", "Scheduler menu closed.");
            break;
        }

        if (choice == 4) {
            printf("\nComing in a future sprint.\n");
            scheduler_menu_pause();
            continue;
        }

        if (choice == 1) {
            char name[128] = "";
            char cmd[256] = "";
            char sched_time[128] = "";

            printf("Enter Task Name: ");
            fflush(stdout);
            if (fgets(name, sizeof(name), stdin) != NULL) {
                name[strcspn(name, "\n")] = '\0';
            }

            printf("Enter Command to run: ");
            fflush(stdout);
            if (fgets(cmd, sizeof(cmd), stdin) != NULL) {
                cmd[strcspn(cmd, "\n")] = '\0';
            }

            printf("Enter Scheduled Time (YYYY-MM-DD HH:MM:SS): ");
            fflush(stdout);
            if (fgets(sched_time, sizeof(sched_time), stdin) != NULL) {
                sched_time[strcspn(sched_time, "\n")] = '\0';
            }

            int id = scheduler_task_create(name, cmd, sched_time);
            if (id > 0) {
                printf("\nTask created successfully with ID: %d\n", id);
            }
            scheduler_menu_pause();
        } else if (choice == 2) {
            scheduler_task_list();
            scheduler_menu_pause();
        } else if (choice == 3) {
            char buf[128];
            int delete_id = 0;
            printf("Enter Task ID to delete: ");
            fflush(stdout);
            if (fgets(buf, sizeof(buf), stdin) != NULL) {
                delete_id = atoi(buf);
            }
            scheduler_task_delete(delete_id);
            scheduler_menu_pause();
        }
    }
}
