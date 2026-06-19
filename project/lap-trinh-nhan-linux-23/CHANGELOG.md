# Changelog

All notable changes to this project will be documented in this file.

## Phase 2: Scope Refactoring (Scope Cleanup)

### Removed
- Permanently removed the obsolete modules: **Signal Manager**, **Scheduler**, **Time Manager**, **System Information**, and **Log Viewer**.
- Deleted all obsolete source directories: `modules/signal/`, `modules/scheduler/`, `modules/time/`, `modules/systeminfo/`, and `modules/logviewer/`.
- Deleted deprecated headers: `include/signal_mgr.h`, `include/scheduler.h`, `include/time_mgr.h`, `include/system_info.h`, and `include/log_viewer.h`.
- Deleted obsolete test files and targets: `tests/scheduler_test.c`, `tests/time_test.c`, `tests/system_info_test.c`.
- Deleted obsolete documentation files: `docs/scheduler.md`, `docs/time_manager.md`, `docs/system_info.md`.
- Removed all deprecated visual menu items and legacy routes in `app/main.c` and `app/menu.c`.

### Added / Refactored
- Ported Scheduler database structures and task management directly inside `modules/shell/shell_mgr.c`, keeping them private to the Shell Automation feature.
- Ported Time Manager current time and timezone calculation logic directly inside `modules/shell/shell_mgr.c` as a native helper for the Shell Time Configuration feature.
- Integrated Signal Settings options natively inside `modules/process/process_mgr.c`.
- Updated enums in `include/common.h` and target dependencies in `Makefile`.

## Shell Sprint 4

### Added
- Implemented the Automation Manager submenu inside [modules/shell/shell_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/shell/shell_mgr.c) enabling task automation checks.
- Enabled Option 4 "Automation Manager" in the main Shell Manager menu, making all TUI submenu choices fully functional.
- Configured Option 1 "View Current Crontab" to run `crontab -l` using the existing fork + execvp + waitpid process execution framework, safely listing configured tasks.
- Implemented cron translation parser (`translate_cron()`) inside Option 2 "Explain Cron Expression" to breakdown time fields and display human-readable summaries (e.g. for `0 8 * * 1-5` and `*/5 * * * *`).
- Configured Option 3 "Sample Scheduled Tasks" to show read-only, educational cron job samples without performing system modifications.
- Integrated logging events for automation tasks, tracing cron runs (`Crontab viewed`), parses (`Cron expression parsed`), example selections (`Example selected`), and exit errors (`Errors`).
- Created unit test suite [tests/automation_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/automation_test.c) and added compilation target `test-automation` in [Makefile](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/Makefile).
- Updated Section 10 inside developer documentation [docs/shell_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/shell_manager.md) detailing Cron fields, scheduling samples, and sandbox safety.

## Shell Sprint 3

### Added
- Implemented the Environment Manager submenu inside [modules/shell/shell_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/shell/shell_mgr.c).
- Enabled Option 3 "Environment Manager" in the Shell Manager submenu.
- Configured local environment variable modifications using standard POSIX APIs `getenv()`, `setenv()`, and `unsetenv()`.
- Implemented robust error boundaries and logging for environment operations, tracing inspections (`Variable queried`), set events (`Variable set`), deletions (`Variable removed`), and system errors (`Errors`).
- Created unit verification test [tests/environment_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/environment_test.c) asserting variable lookups, temporary overrides, and deletions.
- Configured Make rules (`test-environment`) inside the root [Makefile](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/Makefile).
- Updated Section 9 inside developer documentation [docs/shell_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/shell_manager.md) detailing inheritance, isolation, and security.

## Shell Sprint 2

### Added
- Implemented the Script Runner subsystem inside [modules/shell/shell_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/shell/shell_mgr.c) enabling safe script executions.
- Enabled Option 2 "Run Script" in the Shell Manager interactive submenu and added a selection menu for running predefined administrative scripts.
- Created the educational script library in `scripts/` containing:
  * [backup.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/scripts/backup.sh) (Dry-run simulated configuration backup)
  * [disk_usage.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/scripts/disk_usage.sh) (Mounted root filesystem check and top-5 logs size scanner)
  * [show_date.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/scripts/show_date.sh) (System timestamp and timezone lookup)
- Configured direct execution of scripts by invoking `/bin/bash` with the script path through `execvp()`, completely reusing the existing low-level process lifecycle framework.
- Standardized logging events specifically for script runs, covering selections (`Script selected`), PID allocations (`Child process created`), executions (`Script executed`), exit states (`Exit status`), and command-not-found codes (`Errors` / `127`).
- Created diagnostic verification script [tests/script_runner_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/script_runner_test.c) and registered the compilation targets.
- Appended Section 8 to developer documentation [docs/shell_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/shell_manager.md) detailing architecture, scripts lifecycles, and flowcharts.

## Shell Sprint 1

### Added
- Implemented Shell Manager core launcher in [modules/shell/shell_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/shell/shell_mgr.c) and [include/shell_mgr.h](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/include/shell_mgr.h) utilizing `fork()`, `execvp()`, and `waitpid()`.
- Integrated Option 13 "Shell Manager" into the Main Menu in [app/main.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/app/main.c) and [app/menu.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/app/menu.c).
- Provided interactive TUI submenu for Shell Manager supporting command execution and Return handling (both options 0 and 5).
- Added system logging checkpoints covering command inputs (`Command entered`), child fork execution (`Child process created`), successful executions (`Command executed`), exit codes (`Exit status`), and error boundaries (`Errors`).
- Implemented developer reference documentation [docs/shell_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/shell_manager.md) and expanded process API reference in [docs/linux_api_reference.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/linux_api_reference.md).
- Created a test harness in [tests/shell_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/shell_test.c) and added compilation/cleanup workflows in root [Makefile](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/Makefile).
## Kernel Sprint 3

### Added
- Implemented the user-space integration manager in [modules/kernel/kernel_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/kernel/kernel_mgr.c) and [include/kernel_mgr.h](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/include/kernel_mgr.h).
- Added Option 10 "Kernel Module" in [app/main.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/app/main.c) dispatch loop routing to Kernel Module submenu.
- Configured robust reading of `/proc/sysmgr` using standard library `fopen()`, `fgets()`, and `fclose()` functions.
- Implemented graceful error handling and system log tracking for request initialization (`Kernel module information requested`), read success (`Read success`), and read failure (`Read failure`) when the module is unloaded/missing.
- Created automated integration test suite in [tests/kernel_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/kernel_test.c) and added target `test-kernel` to Makefile.
- Expanded Developer Guide in [docs/kernel_module.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/kernel_module.md) detailing the user-space integration architecture and sequence flow diagram.

## Kernel Sprint 2

### Added
- Implemented read-only `/proc/sysmgr` virtual file interface using `seq_file` and `proc_ops` in [kernel/system_monitor/system_monitor.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/kernel/system_monitor/system_monitor.c).
- Configured dynamic retrieval of kernel module name (`KBUILD_MODNAME`), active kernel release (`init_utsname()->release`), current `jiffies` timer, and formatted module loading calendar timestamp.
- Verified that `/proc/sysmgr` is created cleanly during module initialization and safely unregistered via `remove_proc_entry()` during module exit to prevent descriptor leaks.
- Documented VFS user-space-to-kernel-space execution flow, `seq_file` callbacks, and `proc_ops` hooks in [docs/kernel_module.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/kernel_module.md).

## Kernel Sprint 1

### Added
- Created top-level module driver orchestrations in [kernel/Makefile](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/kernel/Makefile) supporting build, clean, insmod, and rmmod commands.
- Implemented LKM initialization (`module_init`), cleanup (`module_exit`), licensing, descriptors metadata, log verbosity parameters, and printk logging inside [kernel/system_monitor/system_monitor.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/kernel/system_monitor/system_monitor.c).
- Integrated load and unload targets into [kernel/system_monitor/Makefile](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/kernel/system_monitor/Makefile) using `M=$(shell pwd)` for recursive Kbuild module compilation.
- Created developer reference documentation [docs/kernel_module.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/kernel_module.md) detailing LKM lifecycles, Kbuild, and loading/unloading verification.

## Socket Sprint 2

### Added
- Implemented multi-client TCP Echo Server in [modules/socket/socket_multi_server.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/socket/socket_multi_server.c) separating Server, Session, and Protocol layers.
- Equipped multi-client server with POSIX thread workers (`pthread_create`) and automatic thread recycling (`pthread_detach`), preventing resource leaks.
- Integrated option 3 (Multi Client Echo) in [modules/socket/socket_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/socket/socket_mgr.c) routing to multi-client server start.
- Rewrote [tests/socket_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/socket_test.c) to fork 3 concurrent clients in parallel, assert separate session echoes, and cleanly tear down server.
- Documented session thread architectures and lifecycle states in [docs/socket_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/socket_manager.md).

## Socket Sprint 1

### Added
- Implemented single-connection TCP Echo Server in [modules/socket/socket_server.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/socket/socket_server.c) using standard POSIX APIs.
- Implemented TCP Client loop in [modules/socket/socket_client.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/socket/socket_client.c) to send strings and receive echoed packets.
- Implemented TUI submenu layout for Socket Manager in [modules/socket/socket_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/socket/socket_mgr.c).
- Integrated Main Menu Option 5 in [app/main.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/app/main.c) to route control to [socket_mgr_run()](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/socket/socket_mgr.c).
- Created automated check [tests/socket_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/socket_test.c) asserting handshake connectivity and echo correctness.
- Generated developer guide [docs/socket_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/socket_manager.md) explaining socket lifecycles, states, and sequence flows.

## Release Candidate Sprint (v0.7.0-rc1)

### Changed
- Standardized and synchronized `APP_VERSION` across [include/config.h](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/include/config.h), [README.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/README.md), and [PROJECT_STATUS.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/PROJECT_STATUS.md) to `v0.7.0-rc1`.
- Performed verification of all sub-menus (File, Process, Network, Package, Time, Scheduler) to confirm Option 0 Return behavior.
- Executed full test suites for all 8 subsystems (`logger`, `file`, `process`, `network`, `package`, `time`, `scheduler`) and verified successful zero-error, zero-warning status.

## Scheduler Sprint 1

### Added
- Implemented Scheduler in-memory task database in [modules/scheduler/scheduler.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/scheduler/scheduler.c) containing Task ID, Name, Command, Scheduled Time, Status, and Creation Time.
- Implemented `scheduler_task_create()`, `scheduler_task_list()`, and `scheduler_task_delete()` APIs in [modules/scheduler/scheduler.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/scheduler/scheduler.c).
- Connected interactive submenu loop in `scheduler_run()` with choices to create, list, delete tasks, and exit back to main menu.
- Routed Main Menu Option 7 in [app/main.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/app/main.c) to execute `scheduler_run()`.
- Created automated checks inside [tests/scheduler_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/scheduler_test.c) covering creation, deletion, list verification, and invalid boundaries.
- Created [docs/scheduler.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/scheduler.md) documenting task lifecycles, data structures, and future execution mechanisms.
- Configured log diagnostic alerts (`Task created`, `Task removed`, `Task listed`, `Errors`) for the scheduler transactions.

## UI Standardization Sprint

### Changed
- Standardized all application menus so that `0` is consistently used for Exit (Main Menu) and Return (all submenus).
- Updated Main Menu selection validation and loop routing in [app/main.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/app/main.c) and display in [app/menu.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/app/menu.c).
- Updated Process Manager submenu options, validation, and loop routes in [modules/process/process_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/process/process_mgr.c).
- Updated Network Manager submenu options, validation, and loop routes in [modules/network/network_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/network/network_mgr.c).
- Updated Package Manager submenu options, validation, and loop routes in [modules/package/package_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/package/package_mgr.c).
- Updated Time Manager submenu options, validation, and loop routes in [modules/time/time_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/time/time_mgr.c).
- Updated documentation flowcharts in [docs/time_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/time_manager.md) to reflect Option 0 return logic.

## Time Manager Sprint 1

### Added
- Implemented the Time Manager module in [modules/time/time_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/time/time_mgr.c) using direct Linux clock/system APIs (`time()`, `localtime_r()`, `gmtime_r()`, `strftime()`, `tzset()`, `sysinfo()`).
- Added an interactive submenu loop in `time_mgr_run()` with choices to display system time or return to the main menu.
- Integrated the Time Manager module (Option 11) into the main application menu loop (`app/main.c` and `app/menu.c`).
- Created unit tests in [tests/time_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/time_test.c) to verify local time formatting, UTC formatting, Unix epoch, timezones, and system uptime.
- Created [docs/time_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/time_manager.md) documenting system clock APIs, timezone handling, epoch calculations, and execution flows.
- Configured log diagnostic alerts (`Time inspection started`, `Current timestamp retrieved`, `Timezone detected`, `Errors`, `Leaving Time Manager`) for the time operations.

## Package Manager Sprint 3

### Added
- Implemented `package_mgr_info()` in [modules/package/package_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/package/package_mgr.c) to inspect detailed package metadata (Name, Version, Release, Architecture, Vendor/Maintainer, License, Install Date, Summary, and Description).
- Added static safety helper `is_safe_package_name()` to validate inputs before executing commands, preventing shell injection vulnerabilities.
- Connected Submenu Option 5 to prompt for package names and display normalized metadata.
- Configured log diagnostic alerts (`Package selected`, `Metadata query started`, `Success`, `Errors`) inside `package_mgr_info()`.
- Expanded unit tests in [tests/package_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/package_test.c) to verify info lookups for existing packages, missing packages, and unsafe inputs.
- Updated [docs/package_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/package_manager.md) to document Sprint 3 metadata fields, read-only query strategy, validation protocols, and execution flows.

## Package Manager Sprint 2

### Added
- Implemented `package_mgr_search()` in [modules/package/package_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/package/package_mgr.c) using `strcasestr()` for case-insensitive partial substring matches.
- Connected Submenu Option 2 to prompt for keywords and execute `package_mgr_search()`.
- Expanded [tests/package_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/package_test.c) to verify search results for existing, missing, case-insensitive, and partial package matches.
- Updated [docs/package_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/package_manager.md) to detail search algorithms, case-insensitive rules, performance overheads, and future native library migration routes.

## Package Manager Sprint 1

### Added
- Implemented Package manager system detection (`dpkg` and `rpm`) using `access()`.
- Implemented read-only package enumeration and counting in [modules/package/package_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/package/package_mgr.c) using `popen()` / `pclose()` and standard parsing.
- Created interactive Package Manager submenu with 6 choices, linking Option 1 (List Installed Packages) and return paths, and stubbing out future items.
- Created [tests/package_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/package_test.c) to verify package manager detection, listing generation, and total package count.
- Created [docs/package_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/package_manager.md) documenting supported package databases and query architectures.

### Changed
- Connected Option 6 (Package Manager) in the Main Menu (`app/main.c`) to launch the Package Manager submenu.
- Updated root [Makefile](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/Makefile) to build and clean `test-package`.

## Network Manager Sprint 3

### Added
- Implemented Host Information diagnostics in [modules/network/network_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/network/network_mgr.c) using `gethostname()`, `getaddrinfo()` (AI_CANONNAME FQDN resolution), and `uname()`.
- Connected Submenu Option 1 to execute `network_show_host_info()`.
- Updated [tests/network_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/network_test.c) to execute host configuration diagnostic test.
- Updated [docs/network_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/network_manager.md) to document Hostname, FQDN, Kernel Node Name, and Domain Name resolving.

## Network Manager Sprint 2

### Added
- Implemented Routing Information parser and Default Gateway detector in [modules/network/network_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/network/network_mgr.c) by parsing `/proc/net/route`.
- Updated Network Manager submenu loop structure with 6 option entries, stubbing future features with "Coming in a future sprint." and connecting Interface and Routing details.
- Updated [tests/network_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/network_test.c) to execute both interface diagnostics and routing information tests.
- Updated [docs/network_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/network_manager.md) to explain `/proc/net/route` formats, gateway detection algorithms, and routing examples.

### Changed
- Refined routing parser flags format to decode standard flags (U, G, H).

## Network Manager Sprint 1

### Added
- Implemented network interface discovery and diagnostic panel in [modules/network/network_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/network/network_mgr.c).
- Added an interactive submenu loop in `network_mgr_run()` with choices to list interfaces or return to the main menu.
- Created [tests/network_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/network_test.c) to verify interface enumeration, status, MTU, MAC addresses, IPv4, and IPv6 info configuration.
- Created [docs/network_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/network_manager.md) documenting design, Linux API listings, flowcharts, and error handling.

### Changed
- Connected Option 4 in the Main Menu (`app/main.c`) to launch the Network Manager submenu.
- Cleaned up legacy unused network functions from [include/network_mgr.h](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/include/network_mgr.h) and [modules/network/network_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/network/network_mgr.c).
- Updated root [Makefile](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/Makefile) to build and clean `test-network`.

## Core Integration Sprint (Version v0.5)

### Added
- Integrated all feature-complete modules (File Manager, Process Manager, System Information) into the main application.
- Connected Option 8 in the main application menu to dispatch control to `system_info_run()`.
- Connected Option 10 in the Process Manager TUI menu to execute the `daemon_demo_run()` worker.

### Changed
- Refactored `app/main.c` choice validation and loop dispatch.
- Removed remaining `TODO` stub printout for Daemon Demo in `modules/process/process_mgr.c`.
- Updated central system documentation ([README.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/README.md), [docs/architecture.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/architecture.md)) to reflect Version v0.5 and the module integration structure.

## Build System Refactor Sprint

### Added
- Independent compilation support for all Process Manager lifecycle demonstrations (`fork_demo.c`, `exec_demo.c`, `wait_demo.c`, `zombie_demo.c`, `orphan_demo.c`, `daemon_demo.c`) into standard `.o` object files.
- Created [docs/architecture.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/architecture.md) explaining the multi-object build strategy, complete with comparison build graphs.

### Changed
- Refactored [modules/process/process_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/process/process_mgr.c) to remove all direct `.c` file inclusions (`#include "demo/*.c"`).
- Updated [modules/Makefile](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/Makefile) and Root [Makefile](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/Makefile) to compile and link separate object files correctly.

## Process Sprint 4F (Daemon Process Demonstration)

### Added
- Implemented Linux Daemon Process Demonstration in [modules/process/demo/daemon_demo.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/process/demo/daemon_demo.c).
- Implemented directory-swapping wrappers `daemon_log_info` and `daemon_log_error` in [modules/process/demo/daemon_demo.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/process/demo/daemon_demo.c) to temporarily switch working directory back to `WORKSPACE_PATH` during log writes, resolving relative path resolution failure to `logs/system.log` after executing `chdir("/")`.
- Documented daemon concepts (double fork, setsid, umask, chdir, standard fd closure, difference between background and daemon processes) and added ASCII sequence diagram in [docs/process_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/process_manager.md).

### Changed
- Replaced non-functional Daemon Demo Stub with full functional interactive/automated demonstration implementation.
- Refined grandparent process status check in [modules/process/demo/daemon_demo.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/process/demo/daemon_demo.c) to correctly verify clean termination (state `'Z'`) of the daemon under sandbox container environments where adopted processes might not be aggressively reaped by PID 1.

## Process Completion Sprint

### Added
- Integrated process demonstrations (Fork, Exec, Wait, Zombie, and Orphan) into the interactive Process Manager TUI menu choice dispatcher in [modules/process/process_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/process/process_mgr.c).
- Integrated Daemon demo stub check in [tests/process_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/process_test.c).
- Expanded [docs/process_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/process_manager.md) with overview details, flowcharts, public API reference table, error scenarios, and known limitations.

### Changed
- Removed "(TODO)" labels from implemented menu features (Fork, Exec, Wait, Zombie, Orphan) and linked them in the interactive execution switch statement.

### Fixed
- Fixed TUI options rendering logic and resolved compiler warning concerns.

### Notes
- Adhered strictly to the negative constraint of not implementing the actual Daemon background loop.

## Process Sprint 4E

### Added
- Implemented Orphan Process Demonstration in [modules/process/demo/orphan_demo.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/process/demo/orphan_demo.c).
- Integrated `orphan_demo_run` into [tests/process_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/process_test.c).
- Documented orphan process theory, sub-reaper behaviors, and differences from zombie states in [docs/process_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/process_manager.md).

### Changed
- None.

### Fixed
- None.

### Notes
- Uses robust sleep loop tracking to prevent signal interruptions and gracefully detects re-parenting events.

## Process Sprint 4D

### Added
- Implemented Zombie Process Demonstration in [modules/process/demo/zombie_demo.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/process/demo/zombie_demo.c).
- Integrated `zombie_demo_run` into [tests/process_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/process_test.c).
- Created detailed developer documentation in [docs/process_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/process_manager.md).

### Changed
- None.

### Fixed
- None.

### Notes
- Uses low-level filesystem I/O (`open()`, `read()`, `close()`) to verify the state transitions of the child process under `/proc/<pid>/status`.

## Sprint Core-Foundation

### Added
- Created unified error code header [include/error.h](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/include/error.h) containing `err_code_t` enum and `err_to_string()` mapper.
- Created string utility module [modules/core/string_util.h](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/core/string_util.h) and [modules/core/string_util.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/core/string_util.c) with stub declarations.
- Created time utility module [modules/core/time_util.h](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/core/time_util.h) and [modules/core/time_util.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/modules/core/time_util.c) with stub declarations.
- Documented foundation architecture in [docs/core_foundation.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/core_foundation.md).

### Changed
- Refactored [include/common.h](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/include/common.h) to define type aliases, boolean flags, and module enum IDs.
- Updated [include/config.h](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/include/config.h) with app metadata, buffer limits, default paths, and socket port variables.
- Modified `modules/Makefile` to build and link `core/string_util.o` and `core/time_util.o` objects into the project binary.

### Fixed
- Verified clean build verification with zero compilation warnings.

### Notes
- Kept utilities as stub declarations with TODO comments, adhering strictly to non-functional foundation requirements.

## Sprint Network-2

### Added
- Standard developer documentation in [docs/network_manager.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/network_manager.md).
- Diagnostic test coverage in [tests/network_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/network_test.c).
- Internal static helper `read_entire_file()` to handle arbitrary size file reading using `open()`, `read()`, and `close()`.

### Changed
- Replaced stub functions in `modules/network/network_mgr.c` with POSIX-compliant file-reading parsers for default gateway, DNS servers, routing table, and interface statistics.

### Fixed
- Included `<fcntl.h>` to resolve implicit declaration errors for `open()` and `O_RDONLY`.

### Notes
- Strictly used low-level POSIX I/O (`open()`, `read()`, `close()`). Standard library stream operations like `fopen()` are avoided.
