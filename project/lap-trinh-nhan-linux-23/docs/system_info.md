# System Information Module - Sprint 1 Developer Documentation

## 1. Purpose
The purpose of the System Information Module is to gather and display basic system information from a running Linux host, providing a clear educational demonstration of standard Linux APIs and low-level system calls.

## 2. Linux APIs Used
* **`uname()`**: Queries kernel name, release, version, machine architecture, and node name from the system kernel.
* **`gethostname()`**: Retrieves the system's network hostname.
* **`open()` / `read()` / `close()`**: Low-level POSIX file system calls used to open, read, and close `/etc/os-release` to extract the pretty-printed Operating System name.

## 3. Directory Layout
```
modules/systeminfo/
├── Makefile
└── system_info.c
include/
└── system_info.h
tests/
└── system_info_test.c
docs/
└── system_info.md
```

## 4. Public APIs
All public APIs are documented below:

### `void system_info_run(void)`
* **Purpose**: Orchestrates the diagnostic run, logging the start of the query and sequentially executing all child information fetchers.
* **Linux APIs used**: None directly (acts as a dispatcher).
* **Parameters**: None.
* **Return value**: None.

### `int system_info_kernel(void)`
* **Purpose**: Retrieves and prints the kernel name, kernel release, and kernel version.
* **Linux APIs used**: `uname()`
* **Parameters**: None.
* **Return value**: `0` on success, `-1` on failure.

### `int system_info_hostname(void)`
* **Purpose**: Retrieves and prints the system hostname.
* **Linux APIs used**: `gethostname()`
* **Parameters**: None.
* **Return value**: `0` on success, `-1` on failure.

### `int system_info_os(void)`
* **Purpose**: Parses `/etc/os-release` and prints the operating system's pretty name.
* **Linux APIs used**: `open()`, `read()`, `close()`
* **Parameters**: None.
* **Return value**: `0` on success, `-1` on failure.

### `int system_info_architecture(void)`
* **Purpose**: Retrieves and prints architecture, machine hardware name, and node name.
* **Linux APIs used**: `uname()`
* **Parameters**: None.
* **Return value**: `0` on success, `-1` on failure.

---

## 5. Internal Implementation
* **OS Parsing**: Reads `/etc/os-release` into a local buffer of `2048` bytes using `open()` and `read()`. It manually parses the configuration line-by-line using pointer arithmetic and `strncmp`, checking for the `PRETTY_NAME=` line (and falling back to `NAME=` if `PRETTY_NAME` is absent). It strips double or single quotes surrounding the string value to produce clean, user-friendly output.
* **Kernel & Hostname Queries**: Directly makes the `uname` and `gethostname` system calls, extracting the relevant fields from `struct utsname` and local character buffers respectively.

## 6. Execution Flow
```
system_info_run()
  ├── Log: "System information requested"
  ├── system_info_hostname()       ──> Calls gethostname()
  ├── system_info_os()             ──> Reads and parses /etc/os-release
  ├── system_info_kernel()         ──> Calls uname() (sysname, release, version)
  └── system_info_architecture()   ──> Calls uname() (machine, nodename)
```

## 7. Error Handling
* **uname/gethostname failure**: If the system call fails, the module logs the API failure including `errno`, prints `Unknown` for the affected fields, and exits the function with `-1`.
* **Missing `/etc/os-release`**: If the file does not exist (`ENOENT`), the module logs the specific missing file warning, prints `Unknown`, and continues execution cleanly.
* **Permission Denied**: If opening `/etc/os-release` fails with `EACCES`, the module logs the permission denied warning, prints `Unknown`, and continues execution.
* **Robustness Guarantee**: Errors in any single query never terminate the application or halt downstream system info calls.

## 8. Test Cases
Tests are implemented in `tests/system_info_test.c` to:
1. Call `system_info_run()`.
2. Inspect the terminal output and verify the printed fields match `uname -a` and `hostnamectl` command-line utilities.

## 9. Example Output
```
=== System Information (Sprint 1) ===
Hostname:          sandbox
Operating System:  Fedora Linux 44 (Workstation Edition)
Kernel Name:       Linux
Kernel Release:    7.0.12-201.fc44.x86_64
Kernel Version:    #1 SMP PREEMPT_DYNAMIC Thu Jun 11 01:30:16 UTC 2026
Architecture:      x86_64
Machine:           x86_64
Node Name:         sandbox
=====================================
```

## 10. Future Improvements
* **Sprint 2**: Integrate hardware metrics (CPU details from `/proc/cpuinfo`, Memory/Swap capacities, and disk utilization statistics).
* **Sprint 3**: Add uptime tracker and identify the currently logged-in user.
