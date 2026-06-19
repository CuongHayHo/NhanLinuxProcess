# Task Scheduler Module

This document describes the data structures, lifecycle rules, in-memory architectures, and future execution plans for the **Task Scheduler** module.

---

## 1. Purpose
The Task Scheduler module demonstrates scheduling concepts under **Learning Mode** (Read-Only). It provides an in-memory task database supporting safe interactive job creation, listing, and deletion without executing background scripts or modifying system crontabs.

---

## 2. Data Structures
The task scheduler relies on an in-memory task storage array defined in [include/scheduler.h](file:///include/scheduler.h):

```c
#define MAX_TASK_NAME 64
#define MAX_TASK_CMD 128
#define MAX_TIME_STR 32
#define MAX_TASKS 64

typedef struct {
    int id;                                // Auto-incrementing unique task ID
    char name[MAX_TASK_NAME];              // User-facing mnemonic name
    char command[MAX_TASK_CMD];            // Target command string
    char scheduled_time[MAX_TIME_STR];     // Planned execution date/time
    char status[16];                       // Active execution status ("PENDING")
    char creation_time[MAX_TIME_STR];      // System time at creation
} scheduler_task_t;
```

---

## 3. Task Lifecycle
Each scheduled task moves through the following state changes:

```
[Creation] -> [Status: PENDING] -> [Delete/Reap] -> [Destroyed]
```

*   **Creation**: Instantiated with a unique auto-incrementing ID. Status initialized to `PENDING`. Creation timestamp generated via native `time()` and `strftime()`.
*   **Listing**: Renders scheduled tasks from memory, formatting states into tabular summaries.
*   **Deletion**: Removes tasks from memory, shifting subsequent array elements left to reclaim array slots.

---

## 4. Execution Flow
The Task Scheduler submenu dispatches requests as follows:

```
+-------------------------------------------------------------------------------+
|                               scheduler_run()                                 |
|                              (Interactive Menu)                               |
+-------------------------------------------------------------------------------+
                                 |
         +-----------------------+-----------------------+
         |                       |                       |
         v                       v                       v
 [Option 1: Create]       [Option 2: List]       [Option 3: Delete]
         |                       |                       |
         v                       v                       v
scheduler_task_create()  scheduler_task_list()   scheduler_task_delete()
```

---

## 5. Future Execution Engine
To transition this module into full execution mode, the scheduler could incorporate:
1.  **Background Thread Engine**: An independent POSIX thread (`pthread_create`) checking pending tasks against system time every second.
2.  **Worker Forking**: Utilizing `fork()` and `execve()` inside the worker threads to execute commands when the current system epoch matches `scheduled_time`.
3.  **Job Status Tracking**: Transitioning status to `COMPLETED` or `FAILED` based on child process exit values retrieved via `waitpid()`.
