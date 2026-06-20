# File Manager (Shell Programming Compliance)

This document describes the design, implementation, and execution flow of the **File Manager** module, refactored to comply with the assignment requirement:

> "Lập trình shell để quản lý file."

---

## 1. Compliance Architecture Overview

The File Manager is designed as a C TUI launcher delegating all operations to a Bash backend script. The compiled C code does not contain any direct POSIX file manipulation operations (no `open()`, `read()`, `write()`, `unlink()`, `rename()`, `stat()`, or `readdir()`).

```
+---------------------------------------+
|           file_mgr.c (C)              |
|  - Renders interactive Menu           |
|  - Reads user inputs & paths          |
|  - Spawns subprocesses (fork-exec)    |
|  - Buffers read contents via pipes    |
|  - Checks status & logs results       |
+---------------------------------------+
                    |
                    v (fork + execvp)
+---------------------------------------+
|          shell/file.sh (Bash)         |
|  - Executes raw bash tools:           |
|    touch, cat, echo, rm, mv, cp, stat,|
|    ls, mkdir, chmod, find, tar        |
+---------------------------------------+
```

---

## 2. Interactive Menu Structure (C Front-End)
The File Manager front-end displays a uniform terminal menu interface:
```text
========================================
File Manager
============
1. Create File
2. Read File
3. Write File
4. Delete File
5. Rename File
6. Copy File
7. Move File
8. File Information
9. Directory Listing
10. Create Directory
11. Change Permission
12. Search Files
13. Archive
14. Return
========================================
```

---

## 3. Submenu Functionality & Wrapper Details

All C wrappers fork and execute `/bin/bash shell/file.sh <action> <args...>`:

1.  **Create File**: Launches `create` action $\rightarrow$ `touch "$2"`. Logs `File created`.
2.  **Read File**: Launches `read` action $\rightarrow$ `cat "$2"`. Captures output in C using a pipe.
3.  **Write File**: Launches `write` action $\rightarrow$ `echo -n "$3" > "$2"`. Logs `File created`.
4.  **Delete File**: Launches `delete` action $\rightarrow$ `rm -rf "$2"`. Logs `File removed`.
5.  **Rename File**: Launches `rename` action $\rightarrow$ `mv "$2" "$3"`. Logs `File moved`.
6.  **Copy File**: Launches `copy` action $\rightarrow$ `cp -r "$2" "$3"`. Logs `File copied`.
7.  **Move File**: Launches `move` action $\rightarrow$ `mv "$2" "$3"`. Logs `File moved`.
8.  **File Information**: Launches `info` action $\rightarrow$ `stat "$2"`.
9.  **Directory Listing**: Launches `list` action $\rightarrow$ `ls -lah "$2"`. Logs `Directory listed`.
10. **Create Directory**: Launches `mkdir` action $\rightarrow$ `mkdir -p "$2"`.
11. **Change Permission**: Launches `chmod` action $\rightarrow$ `chmod "$3" "$2"`. Logs `Permission changed`.
12. **Search Files**: Launches `search` action $\rightarrow$ `find "$2" -name "$3"`.
13. **Archive**: Launches `archive` action $\rightarrow$ `tar -czf "$2" "$3"`. Logs `Archive created`.

---

## 4. Tests and Regression Safeguards
Regression tests in `tests/file_test.c` verify the shell backend script, command routing wrappers, directory creation, search queries, metadata parsing, and cleanup. All actions are checked to prevent regressions.
