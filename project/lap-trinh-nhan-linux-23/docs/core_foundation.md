# Core Foundation Specification

This document details the shared project foundation implemented for the Linux System Manager.

## 1. Architecture Overview
The Linux System Manager architecture consists of independent feature modules interacting through unified header boundaries. The Core Foundation provides a shared, reliable layer of helpers and configurations used by all modules:

```
+--------------------------------------------------------+
|                      TUI / app                         |
+--------------------------------------------------------+
                           ||
+--------------------------------------------------------+
|                    Feature Modules                     |
|  (File, Process, Network, Socket, Package, Shell)      |
+--------------------------------------------------------+
                           ||
+--------------------------------------------------------+
|                    Core Foundation                     |
|  - include/common.h  include/config.h  include/error.h |
|  - modules/core/string_util  modules/core/time_util    |
+--------------------------------------------------------+
```

## 2. Shared Headers

### `include/common.h`
Contains project-wide types, enums, boolean helpers, and global constants to ensure type compatibility across different developer implementations.
* **Boolean helpers**: Definitions of `TRUE` and `FALSE` mapping to the C99 standard `bool`.
* **Shared typedefs**: Defines standard aliases like `byte_t` for raw byte arrays and `status_t` for return codes.
* **Common enums**: Defines `module_id_t` listing all modules in the project.
* **Global Constants**: Configures system limits such as `MAX_PATH_LEN` (256), `MAX_BUF_LEN` (1024), and `MAX_LINE_LEN` (512).

### `include/config.h`
Maintains global application configurations, default file paths, buffer constraints, default ports, and log system directories.
* **Identities**: Defines `APP_NAME` and `APP_VERSION`.
* **Default paths**: Configures default paths for files such as `/proc/net/route`, `/proc/net/dev`, `/etc/resolv.conf`, temp folders, and the configuration file path.
* **Buffer sizes**: Standardizes memory layouts like `DEFAULT_BUF_SIZE` (4096).
* **Ports**: Configures default server port boundaries.

### `include/error.h`
Specifies a unified error interface `err_code_t` that modules will gradually migrate to. It maps numeric errors to descriptive strings using `err_to_string()`.

---

## 3. Core Utilities

### String Utilities (`modules/core/string_util.h`)
* `str_trim(char* str)`: Trims leading/trailing whitespaces in-place.
* `str_split(...)`: Safely tokenizes strings with a custom delimiter.
* `str_safe_copy(...)`: Secure bounded copy guaranteeing null-termination.

### Time Utilities (`modules/core/time_util.h`)
* `time_get_current_str(...)`: Returns formatted local system timestamps.
* `time_calc_duration(...)`: Computes millisecond duration differences.
* `time_get_timestamp_ms(...)`: Reads a high-resolution monotonic clock.

---

## 4. Future Usage
Existing and future modules will incrementally refactor their operations to:
1. Replace ad-hoc numeric error returns with the predefined `err_code_t` enum.
2. Delegate string scrubbing, splitting, and path checks to `string_util`.
3. Standardize execution metrics and logging timestamps with the high-resolution clocks in `time_util`.
