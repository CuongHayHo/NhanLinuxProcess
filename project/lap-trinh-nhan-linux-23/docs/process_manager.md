# Process Manager Developer Documentation

This document describes the design, implementation, execution flow, error handling, and API boundaries of the **Process Manager** module.

---

## 1. Module Overview
The Process Manager module collects and displays active Linux process diagnostics, manages process behavior via standard Unix signals and nice priorities, and hosts educational demonstrations illustrating Linux process lifecycle events.

---

## 2. Linux APIs and Data Sources Used
* **`/proc/<pid>/stat`**: Read via POSIX `open()`, `read()`, `close()` to parse PPID, State, and Priority.
* **`/proc/<pid>/status`**: Read to extract memory details (`VmRSS`) and verify Zombie states.
* **`/proc/<pid>/cmdline`**: Read to fetch command-line arguments.
* **`fork()`**: Spawns concurrent child processes.
* **`execvp()`**: Replaces the process address space with a new binary image.
* **`wait()` & `waitpid()`**: Synchronizes process termination and reaps exited children.
* **`kill()`**: Dispatches signals to processes.
* **`setpriority()`**: Modifies process nice values.
* **`getpriority()`**: Queries current nice priority.
* **`getpid()` & `getppid()`**: Retrieves current process and parent process identifiers.

---

## 3. Execution Flow

```
+--------------------------------------------------------------+
|                     process_mgr_run()                        |
|                     (Interactive Menu)                       |
+--------------------------------------------------------------+
                               |
            +------------------+------------------+
            |                  |                  |
            v                  v                  v
    [Option 1 - 4]      [Option 5 - 9]       [Option 10]
           |                   |                  |
           v                   v                  v
   Feature Managers     Demo Executions     Daemon Demo Stub
   - List Processes     - fork_demo_run     - Prints TODO
   - Search Process     - exec_demo_run     - Logs finish
   - Send Signal        - wait_demo_run
   - Set Priority       - zombie_demo_run
                        - orphan_demo_run
```

---

## 4. Public APIs

### `void process_mgr_run(void)`
* **Purpose**: Displays the Process Manager TUI menu and dispatches options.
* **Parameters**: None.
* **Return**: None.

### `int process_mgr_list(void)`
* **Purpose**: Iterates over numerical directories in `/proc` and displays a table of active processes.
* **Parameters**: None.
* **Return**: `0` on success, `-1` on failure.

### `int process_mgr_search(const char* query)`
* **Purpose**: Searches for processes matching a numeric PID or name string (case-insensitive substring match).
* **Parameters**: `query` - PID or name query.
* **Return**: `0` on success, `-1` on failure.

### `int process_mgr_send_signal(pid_t pid, int sig)`
* **Purpose**: Sends a standard signal to the target process.
* **Parameters**: `pid` - target process ID; `sig` - signal number.
* **Return**: `0` on success, `-1` on failure.

### `int process_mgr_set_priority(pid_t pid, int nice_val)`
* **Purpose**: Adjusts the nice value of the target process.
* **Parameters**: `pid` - target process ID; `nice_val` - target nice value (-20 to 19).
* **Return**: `0` on success, `-1` on failure.

---

## 5. Demo Descriptions

* **Fork Demo (`fork_demo_run`)**: Spawns a child process and tracks concurrent execution blocks, explaining PID values and code separation.
* **Exec Demo (`exec_demo_run`)**: Overlay of child process with standard system command execution (`/bin/ls`), highlighting that a successful exec call never returns.
* **Wait Demo (`wait_demo_run`)**: Illustrates parent blocking synchronization until the child finishes sleeping and returns its exit code.
* **Zombie Demo (`zombie_demo_run`)**: Child exits immediately while the parent sleeps. The state shifts to `Z` in `/proc/<pid>/status`, and waitpid is subsequently invoked to reap it.
* **Orphan Demo (`orphan_demo_run`)**: Parent exits immediately, child sleeps, and system sub-reaper (PID 1 or user-session systemd manager) adopts the child.
* **Daemon Demo (`daemon_demo_run`)**: System daemon background worker generation demonstration (Stub).

---

## 6. Error Handling
* **PID validation**: PIDs <= 0 or non-numeric queries are rejected as invalid arguments.
* **ESRCH Errors**: Checked if a query process has terminated or does not exist.
* **EPERM / EACCES Errors**: Captured when raising process priority (reducing nice values) or signaling system processes without root privileges.
* **Low-level I/O robustness**: Reads of volatile `/proc` directories check for race-condition exits and skip missing folders without crashes.

---

## 7. Example Outputs (Orphan Demo)
```text
=== Orphan Process Demonstration ===
Parent PID:    60857
[Parent] Forking the Child process...
[Parent] Exiting immediately to orphan the child...
Child PID:     60858
Original PPID: 60857
  [Child] Sleeping for 4 seconds to wait for parent to exit...
[Grandparent] Parent process (PID 60857) exited.
[Grandparent] Waiting 6 seconds for Grandchild (Orphan) to finish printing...
Current PID:   60858
Current PPID:  3024
New PPID:      3024
Adoption detected
  Explanation: The child has been adopted by init/systemd (PID 3024).
```

---

## 8. Known Limitations
* **Nice raising limitations**: Raising nice priority (negative values) requires superuser privileges (`sudo`). Standard runs will throw `Permission denied`.
* **Daemon Demo state**: The Daemon demonstration is currently compiled as a non-functional stub and will print a TODO.
