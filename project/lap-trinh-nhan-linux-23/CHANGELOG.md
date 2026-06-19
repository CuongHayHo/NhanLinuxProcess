# Changelog

All notable changes to this project will be documented in this file.

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
- Standard developer documentation in [docs/network.md](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/docs/network.md).
- Diagnostic test coverage in [tests/network_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/project/lap-trinh-nhan-linux-23/tests/network_test.c).
- Internal static helper `read_entire_file()` to handle arbitrary size file reading using `open()`, `read()`, and `close()`.

### Changed
- Replaced stub functions in `modules/network/network_mgr.c` with POSIX-compliant file-reading parsers for default gateway, DNS servers, routing table, and interface statistics.

### Fixed
- Included `<fcntl.h>` to resolve implicit declaration errors for `open()` and `O_RDONLY`.

### Notes
- Strictly used low-level POSIX I/O (`open()`, `read()`, `close()`). Standard library stream operations like `fopen()` are avoided.
