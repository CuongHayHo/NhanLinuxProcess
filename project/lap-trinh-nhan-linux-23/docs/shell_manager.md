# Shell Manager Developer Documentation

This document describes the design, implementation, execution flow, security considerations, and API boundaries of the **Shell Manager** module.

---

## 1. Module Overview
The Shell Manager module provides a secure program execution subsystem for the Linux System Manager application. It enables the execution of standard Linux shell commands safely from user input by utilizing low-level POSIX process lifecycle APIs.

The objective is educational, demonstrating how shell environments tokenize commands and manage child processes without resorting to the insecure `system()` C library wrapper.

---

## 2. POSIX APIs Used
* **`fork()`**: Spawns a child process by duplicating the calling process's address space.
* **`execvp()`**: Replaces the child process image with the specified executable file. It uses the `PATH` environment variable to locate command files.
* **`waitpid()`**: Synchronously waits for the state change of a specific child process, preventing zombie processes and harvesting exit codes.

---

## 3. Shell Architecture (fork-exec-wait model)

```
        +-----------------------------------+
        |       Parent: shell_mgr_run()      |
        +-----------------------------------+
                          |
                          | 1. Read command & Tokenize
                          v
        +-----------------------------------+
        |              fork()               |
        +-----------------------------------+
          /                               \
         / (pid == 0)                      \ (pid > 0)
        v                                   v
+-------------------------+       +-----------------------------+
|   Child Process         |       |   Parent Process            |
|-------------------------|       |-----------------------------|
| 2. execvp(args[0], args)|       | 2. waitpid(pid, &status, 0) |
+-------------------------+       +-----------------------------+
        |                                       |
        | (If execvp fails)                     |
        v                                       |
| 3. exit(127)            |                     |
+-------------------------+                     v
                                  +-----------------------------+
                                  | 3. Extract exit code via    |
                                  |    WIFEXITED/WEXITSTATUS    |
                                  +-----------------------------+
                                                |
                                                v
                                  +-----------------------------+
                                  | 4. Log exit status          |
                                  +-----------------------------+
```

---

## 4. Argument Parsing and Tokenization
Standard shell launchers must handle user inputs containing flags and arguments (e.g. `ls -l /var/log`).
Before launching a child program, the input command string is safely parsed:
1. The string is duplicated via `strdup()` to preserve the original query and avoid modifying read-only memory.
2. The copy is split into distinct arguments (tokens) using `strtok()` with space (`" "`) and tab (`"\t"`) separators.
3. The arguments are accumulated in a null-terminated pointer array `char* args[64]`.
4. The first token `args[0]` represents the executable filename, and `args` represents the argument vector passed to `execvp()`.

---

## 5. Public APIs

### `void shell_mgr_run(void)`
* **Purpose**: Renders the Shell Manager TUI submenu loop, manages user selection, prompts for command inputs, and invokes execution.
* **Parameters**: None.
* **Return**: None.

### `int shell_mgr_execute(const char* command)`
* **Purpose**: Tokenizes a raw command string, forks a child process, executes the command via `execvp`, waits for completion, and returns the exit status.
* **Parameters**: `command` - raw command string entered by the user.
* **Return**: Exited command status (0-255) on success, or `-1` on error.

---

## 6. Execution Flow
1. **Interactive Choice**: User selects Option 1 ("Execute Command") in the submenu.
2. **Command Prompt**: The user is prompted to input a command string.
3. **Logging**: The system logs the command string inside `logs/system.log` under the `SHELL` tag.
4. **Tokenization**: The input is split into arguments.
5. **Process Creation**: `fork()` splits execution. The parent logs the child's PID.
6. **Execution**: The child calls `execvp()`. If it fails (e.g., command not found), the child prints an error to `stderr` and exits immediately with status `127`.
7. **Synchronization**: The parent blocks inside `waitpid()` until the child process is terminated.
8. **Logging Results**: The parent reaps the child status and logs the command exit status (or terminations/signals) in `logs/system.log`.

---

## 7. Security Considerations

### Why Avoid `system()`?
The standard C library function `system()` wraps the command inside `/bin/sh -c "command"`. This introduces several critical security vulnerabilities:
1. **Shell Injection**: Metacharacters (like `;`, `&&`, `|`, `>`) are parsed by the subshell, allowing users to chain unauthorized commands.
2. **Path Exploitation**: If the search path is compromised, the shell might execute malicious scripts with elevated privileges.
3. **Privilege Escalation**: `system()` retains setuid/setgid privileges in dangerous ways.

### Our Security Boundaries
1. **No Subshell Wrapper**: By calling `execvp()` directly, command arguments are treated strictly as arguments to the binary, preventing shell injection attacks. Metacharacters like `;` are passed as literal arguments rather than parsed as delimiters.
2. **Child Isolation**: The child process is terminated immediately with `exit(127)` if `execvp()` fails. This prevents the child from returning to the parent loop and reading standard input or executing parent instructions.
3. **Strict Bounds Checking**: Tokenization is capped at 63 arguments to prevent memory overflows.
4. **No Zombie Accumulation**: Every spawned child is guaranteed to be reaped by `waitpid()` before the parent resumes the interactive loop.
