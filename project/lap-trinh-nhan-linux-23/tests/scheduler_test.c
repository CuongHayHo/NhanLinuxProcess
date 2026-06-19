/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/scheduler_test.c
 * Purpose: Diagnostic unit tests for Task Scheduler.
 */

#include <stdio.h>
#include <assert.h>
#include "scheduler.h"

int main(void) {
    printf("Starting Task Scheduler Module - Sprint 1 (Task Lifecycle) test program...\n\n");

    printf("--- Test 1: Empty Scheduler Listing ---\n");
    int count = scheduler_task_list();
    printf("Initial list task count: %d\n", count);
    assert(count == 0);

    printf("\n--- Test 2: Task Creation ---\n");
    int id1 = scheduler_task_create("Task A", "echo 'Hello'", "2026-06-20 08:00:00");
    int id2 = scheduler_task_create("Task B", "ls -l /tmp", "2026-06-20 09:30:00");
    printf("Created Task A with ID: %d\n", id1);
    printf("Created Task B with ID: %d\n", id2);
    assert(id1 > 0);
    assert(id2 > 0);
    assert(id1 != id2);

    printf("\n--- Test 3: Scheduler Listing (2 tasks) ---\n");
    count = scheduler_task_list();
    printf("Task count after creations: %d\n", count);
    assert(count == 2);

    printf("\n--- Test 4: Delete Task A (ID %d) ---\n", id1);
    int del_res = scheduler_task_delete(id1);
    assert(del_res == 0);

    printf("\n--- Test 5: Scheduler Listing (1 task remaining) ---\n");
    count = scheduler_task_list();
    printf("Task count after deletion: %d\n", count);
    assert(count == 1);

    printf("\n--- Test 6: Delete Non-existent Task (ID 999) ---\n");
    del_res = scheduler_task_delete(999);
    printf("Deletion of non-existent task returned: %d\n", del_res);
    assert(del_res == -1);

    printf("\n--- Test 7: Clean Up Remaining Task B (ID %d) ---\n", id2);
    del_res = scheduler_task_delete(id2);
    assert(del_res == 0);

    printf("\n--- Test 8: Empty Scheduler Verification ---\n");
    count = scheduler_task_list();
    assert(count == 0);

    printf("\nTask Scheduler Module tests completed successfully.\n");
    return 0;
}
