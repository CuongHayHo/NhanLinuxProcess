# Time Manager Module

This document outlines the design, implementation details, Linux clock APIs, timezone resolution systems, and execution flows for the **Time Manager** module.

---

## 1. Purpose
The Time Manager module is designed to demonstrate Linux clock query capabilities, time representations, and timezone offsets in a strict read-only **Learning Mode**. It queries system times and uptime without executing modifications to the system clocks.

---

## 2. Linux Time Architecture & Concepts

### Epoch Time
Linux tracks time as the number of seconds elapsed since the **Unix Epoch** (January 1, 1970, 00:00:00 UTC). This representation is timezone-independent and is returned by low-level system calls as a `time_t` integer.

### UTC vs. Local Time
*   **UTC (Coordinated Universal Time)**: The global time standard. UTC does not change with seasons or location.
*   **Local Time**: Local time is derived by applying local timezone offsets and Daylight Saving Time (DST) rules to UTC.

### Timezone Handling
Linux systems represent timezones through structural offset files (usually under `/usr/share/zoneinfo`) referenced by the `TZ` environment variable. The library function `tzset()` reads `TZ` (or queries the system link `/etc/localtime`) to populate system external variables:
*   `tzname`: Timezone abbreviation strings.
*   `timezone`: Seconds west of UTC.
*   `daylight`: Non-zero if DST rules apply.

---

## 3. Clock and System APIs Used
The module interacts directly with the kernel using standard Linux/POSIX APIs:
*   **`time(NULL)`**: Retrieves the current system epoch timestamp (`time_t`).
*   **`localtime_r()`**: Converts epoch time to broken-down local time (`struct tm`) in a thread-safe manner, exposing timezone zone names (`tm_zone`) and GMT offsets (`tm_gmtoff`).
*   **`gmtime_r()`**: Converts epoch time to broken-down UTC time (`struct tm`) thread-safely.
*   **`strftime()`**: Formats broken-down times into readable strings based on formatting rules.
*   **`tzset()`**: Standard POSIX initializer to sync environment timezone configurations.
*   **`sysinfo()`**: Queries system statistics, fetching system uptime directly from kernel space (in seconds).

---

## 4. Execution Flow
The Time Manager submenu processes selection dispatching as follows:

```
+-------------------------------------------------------------------------------+
|                                time_mgr_run()                                 |
|                              (Interactive Menu)                               |
+-------------------------------------------------------------------------------+
                                       |
                   +-------------------+-------------------+
                   |                                       |
                   v                                       v
         [Option 1: Show Time]                   [Option 0: Return]
                   |                                       |
                   v                                       v
        time_mgr_show_current()                  Exits submenu loop
                   |
                   +-> time() -> Epoch
                   +-> localtime_r() & gmtime_r()
                   +-> sysinfo() -> Uptime & Boot Time
                   +-> Print formatted metadata
```

---

## 5. Example Output
```text
========================================
Time Information
----------------------------------------
Local Date:     2026-06-19
Local Time:     19:09:30
UTC Time:       2026-06-19 12:09:30 UTC
Unix Timestamp: 1781870970
Time Zone:      +07 (UTC+07:00)
Uptime:         0 days, 00:30:31
Boot Time:      2026-06-19 18:38:59
========================================
```

---

## 6. Future Work
*   **Option 2: Time Zone (Future)**: Browse and change timezone context values (read-only previews/interactive timezone selection).
*   **Option 3: Set Time (Preview)**: Safely simulate time adjustments in learning mode.
*   **Option 4: NTP Status (Future)**: Parse `/etc/ntp.conf` or invoke read-only NTP status queries.
