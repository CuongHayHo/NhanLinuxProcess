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
* **Daemon Demo (`daemon_demo_run`)**: Educational demonstration showing traditional daemonization stages (double-forking, new session initialization, standard file descriptor closure, and signal-driven cleanup).

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
* **Container environment adoption**: In containerized environments, the termination of daemon processes might show them in a zombie state ('Z') because the container's PID 1 entry point does not always perform aggressive zombie reaping of adopted grandchild processes.

---

## 9. Daemon Demonstration & Theory

### ASCII Execution Flow

```text
  Parent
    |
  fork()
    |
  Parent exits
    |
  Child (First Child)
    |
  setsid() (New session leader, detached from terminal)
    |
  fork()
    |
  Parent exits (First Child exits)
    |
  Daemon (Second Child)
    |
  umask(0) & chdir("/")
    |
  Close standard FDs (stdin/stdout/stderr -> /dev/null)
    |
  Loop (Heartbeat log writes)
    |
  SIGTERM received
    |
  Exit cleanly
```

### Explanations of Daemon Creation Sequence

1. **Why Double Fork is Used**:
   * The first `fork()` separates the process from the shell/terminal controller, making it an orphan child adopted by init.
   * `setsid()` creates a new session and process group, making this child the session leader. A session leader, however, can still open a controlling terminal (e.g., if it opens a tty device).
   * The second `fork()` creates a grandchild process. Since it is not a session leader, it is physically impossible for this second child (the actual daemon) to ever re-acquire a controlling terminal.

2. **Purpose of `setsid()`**:
   * Detaches the process from its current controlling terminal, session, and process group. It places the process in a new session and process group where it is the leader, freeing it from terminal signals like SIGHUP.

3. **Purpose of `umask(0)`**:
   * Resets the file mode creation mask to `0`, ensuring that any files created by the daemon have the exact permissions requested by the `open()` or `creat()` system calls (without being restricted by inheritance from the parent process's environment mask).

4. **Purpose of `chdir("/")`**:
   * Changes the working directory to the system root. This ensures that the daemon doesn't hold a reference to the directory it started in, preventing the system from unmounting the file system containing that directory if needed.

5. **Why stdin, stdout, and stderr are closed**:
   * To fully detach the daemon from the terminal. If not closed, the daemon would keep these descriptors open, which might block the parent terminal or SSH session from exiting cleanly. They are closed and redirected to `/dev/null` to prevent any unintended output or input errors from halting the daemon.

6. **Difference between a Background Process and a Daemon**:
   * A **Background Process** runs in the background but still belongs to the session of the controlling terminal. If the terminal closes (e.g., shell exits), it receives a `SIGHUP` and terminates (unless explicitly run with `nohup`).
   * A **Daemon** is fully detached from any controlling terminal session, runs in its own session, is adopted by init (PID 1), and continues running continuously in the background independent of user login sessions.
