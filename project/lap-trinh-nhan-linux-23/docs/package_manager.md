# Package Manager

This document describes the design, implementation, execution flow, error handling, and Linux system data sources for the **Package Manager** module.

---

## 1. Purpose
The Package Manager module demonstrates Linux software package query capabilities. The module is configured in **Learning Mode**, meaning it operates in a strict read-only fashion: it is capable of listing, counting, and searching installed software packages, but prevents any operations that would install, remove, or modify package metadata on the host system.

---

## 2. Supported Package Managers and Databases
The module auto-detects the system's package configuration:
1.  **Fedora / RedHat / CentOS (`rpm`)**:
    *   **Tool**: `/usr/bin/rpm`
    *   **Database Directory**: `/var/lib/rpm/` (containing database files like `rpmdb.sqlite` or `Packages`).
2.  **Debian / Ubuntu (`dpkg`)**:
    *   **Tool**: `/usr/bin/dpkg-query`
    *   **Database File**: `/var/lib/dpkg/status` (flat text configuration file).

---

## 3. Linux APIs and Commands Used
*   **`access(path, X_OK)`**: Verifies system executable access on binary query paths (`/usr/bin/dpkg-query` and `/usr/bin/rpm`) without spawning processes.
*   **`popen(command, "r")`**: Spawns a read-only process pipeline to parse package definitions.
    *   *dpkg query*: `/usr/bin/dpkg-query -W -f='${Package} ${Version}\n'`
    *   *rpm query*: `/usr/bin/rpm -qa --qf '%{NAME} %{VERSION}\n'`
*   **`fgets()`**: Sequentially reads query outputs line-by-line.
*   **`strcasestr(haystack, needle)`**: Performs a case-insensitive substring match of the query string against the package name.
*   **`pclose()`**: Terminates the subprocess pipeline and reaps resources.

---

## 4. Search Algorithm & Matching Rules
The search utility is designed to help administrators quickly filter large package listings:
1.  **Case-Insensitivity**: All searches are case-insensitive. A query for `KERNEL` will match package names containing `kernel`, `Kernel`, or `KERNEL`. This is implemented using the standard GNU C library `strcasestr()` function.
2.  **Partial Matching**: Matches are partial. A search query for `ssl` will successfully match `openssl`, `libssl`, or `openssl-devel`.
3.  **Efficiency**: The search traverses the streamed package definitions line-by-line as they are produced by the package query utility, matching text in real-time, preventing large memory overheads.

---

## 5. Performance Considerations & Future Migration
*   **Current Pipeline Overhead**: Launching subprocesses using `popen` introduces shell startup and context switching overhead. While acceptable for educational stubs, querying thousands of packages can take up to a few hundred milliseconds.
*   **Future Migration to Native Libraries**: To achieve sub-millisecond query performance and eliminate shell dependencies, the module could migrate to native C bindings:
    *   *rpm system*: Integrate directly with `librpm` APIs (e.g., `rpmdbOpen`, `rpmdbInitIterator`, `rpmdbNextIterator`).
    *   *dpkg system*: Parse the `/var/lib/dpkg/status` database file directly using memory-mapped files (`mmap`) and fast regex parsing.

---

## 6. Execution Flow

The Package Manager submenu dispatches selections as follows:

```
+-------------------------------------------------------------------------------+
|                               package_mgr_run()                               |
|                               (Interactive Menu)                              |
+-------------------------------------------------------------------------------+
                                 |
         +-----------------------+-----------------------+
         |                       |                       |
         v                       v                       v
 [Option 1: List]        [Option 2: Search]      [Option 5: Information]
         |                       |                       |
         v                       v                       v
package_mgr_list()      package_mgr_search()     package_mgr_info()
         |                       |                       |
         |                       |                       +-> Validate input
         |                       +-> Prompt keyword      +-> Detect PM
         +-> popen() read-only   +-> strcasestr() match  +-> popen() query
         +-> sscanf() line       +-> print matches       +-> parse key-value
         +-> print details       +-> display count       +-> print metadata
```

---

## 7. Example Outputs

### Package Search (Option 2)
```text
Enter package name to search: kernel

Search Results for 'kernel':
Package Name                             Version                       
------------------------------------------------------------------------
kernel-headers                           7.0.6                         
kernel-core                              7.0.12                        
kernel                                   7.0.12                        
------------------------------------------------------------------------
Total Matches Found: 3
```

---

## 8. Error Handling
*   **Empty Queries**: Empty or null strings passed to the search API are rejected immediately, printing an error and returning `-1`.
*   **Subprocess Failures**: If the `popen` pipe fails to open, the module handles the error gracefully, registers an error log, and cleans up open resources.

---

## 9. Future Work
The following features are placeholder menu entries displaying `"Coming in a future sprint."`:
*   **Option 3: Install Package (Preview)**
*   **Option 4: Remove Package (Preview)**

---

## 10. Package Information (Sprint 3)

### Metadata Fields
The following fields are extracted and displayed in a normalized format:
*   **Name**: Package name.
*   **Version**: Version of the package.
*   **Release**: Release version (RPM only; maps to `N/A` on DPKG).
*   **Architecture**: Target system architecture (e.g. `x86_64`, `amd64`).
*   **Vendor / Maintainer**: Package vendor (RPM) or maintainer (DPKG).
*   **License**: Package license (RPM only; maps to `N/A` on DPKG).
*   **Install Date**: Package installation timestamp (RPM only; maps to `N/A` on DPKG).
*   **Summary**: Brief description of the package.
*   **Description**: Detailed description of the package.

### Read-only Query Strategy
*   **RPM Metadata**: Runs `/usr/bin/rpm -q --qf "..." <pkg_name> 2>/dev/null` using `popen()` in read-only mode to extract formatted key-value metadata fields.
*   **DPKG Metadata**: Runs `/usr/bin/dpkg-query -s <pkg_name> 2>/dev/null` and parses the output block line-by-line. Multi-line description continuation lines (starting with spaces) are captured and appended.
*   **Input Validation**: The package name is strictly validated using `is_safe_package_name()` to ensure it only contains characters `[a-zA-Z0-9.\-_+]`. This prevents command injection vulnerabilities when passing the argument to `popen`.

