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

---

## 8. Script Runner Integration

### Overview
In Sprint 2, the Shell Manager was extended with a **Script Runner** subsystem. This subsystem automates administrative workflows by executing shell scripts safely without duplicating the fork/exec execution framework.

### Fork-Exec Architecture for Scripts
The Script Runner reuses the exact same low-level POSIX process duplication APIs:
1. Instead of executing scripts directly (which requires parsing interpreter shebangs or execute permissions at the C level), the runner invokes `/bin/bash` as the executable, passing the target script path as the first argument (e.g. `/bin/bash scripts/backup.sh`).
2. This translates to the execution of `execvp("/bin/bash", argv)` where `argv` contains `{"/bin/bash", "scripts/backup.sh", NULL}`.
3. This architecture maintains the educational objectives of learning how process namespaces are split via `fork()`, replaced via `execvp()`, and synchronized via `waitpid()`.

### Shell Script Lifecycle
```
+-------------------------------------------------------------------+
|                            Script File                            |
| (Created under scripts/, permissions configured with chmod +x)     |
+-------------------------------------------------------------------+
                                  |
                                  v
+-------------------------------------------------------------------+
|                            TUI Select                             |
| (User chooses predefined script or script path in Option 2)       |
+-------------------------------------------------------------------+
                                  |
                                  v
+-------------------------------------------------------------------+
|                        Execution Launch                           |
| (Command tokenized to /bin/bash <path>, fork() and execvp() call)  |
+-------------------------------------------------------------------+
                                  |
                                  v
+-------------------------------------------------------------------+
|                          Child Execution                          |
| (Bash interprets the script file line-by-line; runs read-only)    |
+-------------------------------------------------------------------+
                                  |
                                  v
+-------------------------------------------------------------------+
|                            Termination                            |
| (Parent reaps child PID via waitpid, logs final exit status)       |
+-------------------------------------------------------------------+
```

### Script Execution Flow
1. **Interactive Submenu Select**: User inputs choice `2` to run a script.
2. **Predefined Selection**: The system displays a list of standard educational scripts:
   * `scripts/backup.sh` - Dry-run backup simulation.
   * `scripts/disk_usage.sh` - File system status.
   * `scripts/show_date.sh` - System clock inspection.
3. **Execution Routing**: The choice is mapped to the format `/bin/bash <script_path>` and sent to `shell_mgr_execute()`.
4. **Child Spawning**: A child process is created via `fork()`.
5. **Image Overlay**: The child calls `execvp("/bin/bash", ...)` which executes bash and lets it run the script file.
6. **Parent Reap**: The parent waits inside `waitpid()` until the script finishes execution.
7. **Diagnostics & Logging**:
   * Logs `Script selected: 'scripts/<name>.sh'` on startup.
   * Logs `Child process created (PID: <pid>)` when fork succeeds.
   * Logs `Script executed` and its `Exit status: <status>` when waitpid returns.
   * If the script path is invalid, bash outputs an error to stderr, and exits with code `127`, which is correctly captured and logged as an error by the parent.

---

## 9. Environment Variables and Process Environment

### Overview
Environment variables are key-value string pairs associated with a process. In Sprint 3, the Shell Manager was updated with an **Environment Manager** module allowing inspection, querying, configuration, and deletion of temporary process environment variables.

### Process Environment & Inheritance Model
* **The `environ` Pointer**: Every Linux process contains a pointer to an array of string pointers called `environ` representing the process environment.
* **Inheritance Model**: When a child process is spawned via `fork()`, the child receives a complete copy of the parent process's environment variables. Any modifications to the parent's environment made via `setenv()` or `unsetenv()` *prior* to the `fork()` will be inherited by the child.
* **API Isolation**: Changes made via C library functions like `setenv()` or `unsetenv()` modify the `environ` memory segment locally inside the current application. They do **NOT** propagate back to the parent shell process (e.g. your interactive bash session) or modify system startup files like `~/.bashrc`, `~/.zshrc`, or `/etc/profile`.

```
           +---------------------------------+
           |        Parent Application       |
           |    (setenv("TEST", "val", 1))   |
           +---------------------------------+
                            |
                            | fork()
                            v
           +---------------------------------+
           |          Child Process          |
           | (Inherits "TEST" = "val" environment)
           +---------------------------------+
```

### Execution Flow for Environment Operations
1. **Interactive Submenu Select**: User inputs choice `3` to open the Environment Manager.
2. **List Variables**: Selecting Option 1 iterates over a list of key system environment variables (`PATH`, `HOME`, `USER`, `PWD`, `SHELL`, `LANG`, `TERM`) and displays their values.
3. **Query Variable**: Selecting Option 2 calls `getenv("VAR_NAME")` to inspect the variable's value and logs the transaction.
4. **Set Variable**: Selecting Option 3 prompts for a name and a value, calls `setenv("VAR_NAME", "value", 1)`, and logs `Variable set: 'VAR_NAME' = 'value'`.
5. **Remove Variable**: Selecting Option 4 prompts for a name, calls `unsetenv("VAR_NAME")`, and logs `Variable removed: 'VAR_NAME'`.

### Security Notes
1. **Local Modifiability**: Modifying environment variables locally is safe because it only affects the current execution session and its spawned children. It cannot compromise permanent system config files or settings.
2. **Buffer Bounds Checks**: Inputs for variable names and values are stored in statically allocated, size-checked buffers (`name_buf[128]` and `val_buf[256]`) with `fgets()`, preventing buffer overflows.
3. Input Sanitization**: Variables containing illegal characters (like `=`) are checked and rejected by `setenv()` with errors, which are captured and logged immediately.

---

## 10. Automation Manager (Cron Tasks and Explanations)

### Overview
Task scheduling in Linux is typically managed by the system daemon `cron`. In Sprint 4, the Shell Manager was updated with an **Automation Manager** submenu. This educational subsystem enables administrators to safely learn cron expression formats and view scheduling without modifying system-wide job setups.

### Cron Scheduling Fields
Cron expressions consist of 5 space-separated fields that determine when a command runs:
```
 +----------------.---------------- minute (0 - 59)
 |                +-------------- hour (0 - 23)
 |                |              +------------ day of month (1 - 31)
 |                |              |            +---------- month (1 - 12)
 |                |              |            |          +------- day of week (0 - 7) (Sunday=0 or 7)
 |                |              |            |          |
 *                *              *            *          *
```

* **Special Operators**:
  * `*`: Every match.
  * `,`: Value separator (e.g. `1,2,5`).
  * `-`: Range specifier (e.g. `1-5` for Monday through Friday).
  * `/`: Step values (e.g. `*/5` for every 5 units).

### Automation Examples
1. **Daily Backup Task**:
   * Cron Expression: `0 2 * * *`
   * Command: `/home/user/scripts/backup.sh`
   * Translation: Run at 02:00 AM every day.
2. **Weekly Log Cleanup Task**:
   * Cron Expression: `0 0 * * 0`
   * Command: `/home/user/scripts/cleanup.sh`
   * Translation: Run every Sunday at midnight (00:00).
3. **Monthly Health Report Task**:
   * Cron Expression: `0 0 1 * *`
   * Command: `/home/user/scripts/report.sh`
   * Translation: Run on the 1st of every month at midnight.

### Execution Flow for Automation Manager
1. **Interactive Submenu Select**: User inputs choice `4` to enter the Automation Manager submenu.
2. **View Current Crontab**: Selecting Option 1 calls `shell_mgr_execute("crontab -l")` to print configured cron tasks. If the user doesn't have a crontab set or commands fail, a descriptive notice is printed.
3. **Explain Cron Expression**: Selecting Option 2 prompts the user for a cron expression, parses it into 5 fields, prints a detailed breakdown of each field, and returns a combined human-readable summary.
4. **Sample Scheduled Tasks**: Selecting Option 3 displays static, safe automation examples to learn typical scripts setups without modifying system crontabs.

### Security Notes
1. **Read-Only Enforcement**: Option 1 only performs `crontab -l` (list). Option 3 displays hardcoded strings. The system never executes `crontab -e` or modifies `/etc/cron*` files, preventing unauthorized job registrations.
2. **Cron Parser Protection**: Cron strings are parsed via standard tokenizers with strict field count validation. Expressions with fewer than 5 tokens are rejected gracefully, preventing memory access issues or logic faults.

---

## 11. Time Configuration (Manual and Automatic Synchronization)

### Overview
In Sprint 2 and Sprint 3 (Time Configuration), the module was expanded to support four primary sub-features:
1. **Manual Time Configuration** - supports **Learning Mode** (dry-run simulation printing equivalent command/API) and **Real Mode** (system clock modification using `clock_settime` POSIX API).
2. **Internet Time Synchronization** - synchronizes the system clock immediately via network NTP servers with chronyc/ntpdate fallback.
3. **Enable Automatic Time Synchronization** - configures the system to sync automatically in the background using systemd's `timedatectl`.

### Real System Time Configuration
To set the system time programmatically without relying on insecure shell command execution wrappers like `system("date -s ...")`, the module utilizes:
* **`clock_settime(clockid_t clk_id, const struct timespec *tp)`**: Used with `CLOCK_REALTIME` to set the system-wide real-time clock.
  * **Input Parsing**: The user input date/time string (in format `YYYY-MM-DD HH:MM:SS`) is validated using custom parsing to reject malformed or impossible dates (e.g., leap years and month/day boundaries).
  * **Conversion**: The validated string is converted to a standard `struct tm` structure, which is translated to epoch seconds `time_t` via `mktime()`.
  * **System API Call**: The epoch seconds are assigned to a `struct timespec` and passed directly to the `clock_settime()` system call.

### Kernel Capabilities & Privilege Checks (`CAP_SYS_TIME`)
Modifying the system calendar clock is a highly privileged operation in Linux:
* **Privilege Level**: If the application runs under a standard non-root user account without special capabilities, `clock_settime()` fails immediately, setting `errno` to `EPERM`.
* **Capabilities**: The calling thread must possess the `CAP_SYS_TIME` capability to modify the clock.
* **Error Diagnostics**: The module inspects the `errno` code returned by the system:
  * `EPERM` -> Displays: `"Root privileges or CAP_SYS_TIME are required."` and logs `"Permission denied"`.
  * `EINVAL` -> Displays: `"Invalid date/time."` and logs `"Validation failed"`.

### Immediate Internet Time Synchronization (Option 4)
This feature allows administrators to synchronize the clock with time servers immediately. It implements a robust, order-based fallback strategy using low-level POSIX execution:
1. **First Option (`chronyc makestep`)**: Checks if `chronyc` exists in the system `PATH` (using `access(..., X_OK)` via the `command_exists` utility). If available, it executes `chronyc makestep` to immediately step the system clock.
2. **Second Option (`ntpdate pool.ntp.org`)**: If `chronyc` is unavailable, it checks if `ntpdate` exists in the system `PATH`. If available, it runs `ntpdate pool.ntp.org`.
3. **Unsupported Fallback**: If neither tool is found, it prints `"Immediate Internet synchronization is unavailable on this system."` and logs the sync failure.
4. **Verification**: After execution, the module retrieves the current time, timezone, and logs which utility succeeded.

#### Requesting Synchronization vs. Verified Synchronization
In system administration, executing a synchronization command (such as `chronyc makestep` or `ntpdate`) only represents a **request** for synchronization. The execution of the command may return success (exit status `0`), but the system clock is not guaranteed to be synchronized immediately or successfully because:
* **Network Latency and Transient Packet Loss**: The NTP UDP packages might be blocked by firewalls or lost during transient outages.
* **Server Unreachability**: The target NTP servers might be overloaded or unresponsive, causing the client tool to timeout.
* **Daemon Lag**: The background synchronization daemon (`chronyd` or `systemd-timesyncd`) may take a few seconds to update its state, reference ID, and step the clock.

To address this, the application enforces **Verified Synchronization**:
1. After sending the synchronization command, the application pauses for approximately 2 seconds to allow the NTP daemon to negotiate and apply changes.
2. It then performs an independent verification check by executing `chronyc tracking` (parsing the Reference ID to ensure it is not `0.0.0.0` or `Not synchronised`) or `timedatectl status` (parsing to verify `System clock synchronized: yes`).
3. Success is reported *only* if the verification check explicitly confirms synchronization. Otherwise, a detailed diagnostics panel is displayed with possible failure causes.

### Enable Automatic Time Synchronization (Option 5)
Configures the system's background NTP client:
* **Timedatectl Check**: It first detects the systemd-based `timedatectl` executable. If unavailable, it prints `"Automatic synchronization is not supported."` and exits option 5.
* **NTP Configuration**: If available, it executes `timedatectl set-ntp true` using the standard fork/exec execution pipeline.
* **Clean Status Report**: Instead of dumping raw output from `timedatectl status`, the module redirects the child process's stdout/stderr to a temporary file (`tmp/cmd_output.txt`) using `dup2()`. It then reads the file and parses only specific fields (`NTP service`, `System clock synchronized`, `RTC mode`, `Time Zone`) to display a clean, formatted report to the administrator.

