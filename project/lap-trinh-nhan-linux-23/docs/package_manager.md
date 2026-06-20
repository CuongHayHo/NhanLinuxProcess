# Package Manager (Shell Programming Compliance)

This document describes the design, implementation, and execution flow of the **Package Manager** module, refactored to strictly comply with the assignment requirement: 

> "Lập trình shell để cài đặt/gỡ bỏ các chương trình tự động."

---

## 1. Compliance Architecture Overview

The Package Manager is split into a simple C-based interactive front-end and a robust shell script backend. There is no business logic or package management code in the compiled C binary.

```
+---------------------------------------+
|          package_mgr.c (C)            |
|  - Renders Menu                       |
|  - Reads User Input                   |
|  - Spawns subprocess (fork-exec)      |
|  - Monitors status & handles errors   |
+---------------------------------------+
                    |
                    v (fork + execvp)
+---------------------------------------+
|         shell/program.sh (Bash)       |
|  - Auto-detects Package Manager       |
|  - Implements Package Search          |
|  - Implements Package Information     |
|  - Performs Safe Installation         |
|  - Performs Safe Removal              |
|  - Runs Automated Safe Demonstration   |
+---------------------------------------+
```

---

## 2. Interactive Menu Structure (C Front-End)
The C front-end displays a uniform terminal menu interface:
```text
========================================
Package Manager
===============
1. Search Package
2. Package Information
3. Install Package
4. Remove Package
5. Safe Demonstration
6. Return
========================================
```

*   **Menu & Launcher**: `package_mgr_run()` processes user input, gathers the target package name if needed, and launches the shell script wrapper `shell/program.sh` using `fork()`, `execvp()`, and `waitpid()`.
*   **Error Checking**: Inspects the exit code of `shell/program.sh` to ensure execution succeeded, reporting and logging errors on non-zero exit codes.

---

## 3. Shell Implementation (`shell/program.sh`)
The shell script performs all business logic and package transactions:
1.  **Auto-detection**: Dynamically identifies the host package manager (`dnf`/`rpm` on Fedora/RHEL, `apt`/`dpkg` on Debian/Ubuntu). No vendor-specific code is hardcoded.
2.  **Search**: Runs local queries (`rpm -qa` / `dpkg-query -W`) to search for installed package substrings.
3.  **Information**: Queries local database specifications (`rpm -q` / `dpkg -s`) if installed, with a clean fallback to remote repository queries (`dnf info` / `apt-cache show`) if not installed.
4.  **Install & Remove**: Dispatches transaction scripts under `sudo` to automate software modifications.
5.  **Safe Demonstration**: Conducts a non-destructive lifecycle simulation utilizing only small, non-critical CLI utilities (`hello`, `sl`, `cowsay`, etc.) to verify search, metadata, installation, local status verification, uninstallation, and cleanup.

---

## 4. Safety Constraints
The shell script blocks any transaction involving critical operating system packages:
*   `kernel` / `glibc` / `bash` / `systemd` / `gcc`
*   `dnf` / `rpm` / `python`
*   Desktop environments, firmware, hardware drivers, or multimedia stacks

---

## 5. Non-Destructive Testing
All regression tests (`tests/package_test.c`) verify script existence, search queries, metadata retrieval, and error paths, but **never install or uninstall packages automatically**. The full installation/removal pipeline is only executed interactively under Menu Option 5 (Safe Demonstration), ensuring system integrity during automated test suites.

---

## 6. Logs & Audits
The Package Manager outputs unified trace logs in `logs/system.log`:
*   `Package search`
*   `Package information`
*   `Package install requested`
*   `Package removal requested`
*   `Safe demonstration started`
*   `Package installed`
*   `Package verified`
*   `Package removed`
*   `Verification completed`
*   `Cleanup completed`
