# Network Manager

This document describes the design, implementation, execution flow, error handling, and Linux system data sources for the **Network Manager** module.

---

## 1. Purpose
The Network Manager module serves as a system administration diagnostic tool, allowing users to:
1. Enumerate host network identity parameters (Hostname, Short Hostname, FQDN, Domain Name, Kernel Node Name).
2. Enumerate local network interfaces and query their configuration structures (status, MTU, MAC address, IPv4, and IPv6 assignments).
3. Inspect the active IPv4 routing table and resolve the default gateway.

These operations are performed using direct POSIX system calls and volatile kernel system files under `/proc`, avoiding any external calls to shell tools (`ip`, `hostname`, `hostnamectl`, `route`, or `ifconfig`).

---

## 2. Linux APIs and Data Sources Used

### Host Network Identity
*   **`gethostname()`**: Queries the standard network hostname configuration.
*   **`getaddrinfo()`**: Performs system address translation. Using the `AI_CANONNAME` flag resolves the canonical FQDN (Fully Qualified Domain Name) from configured nameservers or `/etc/hosts`.
*   **`uname()`**: Queries kernel metrics (through `struct utsname`), yielding the system's `nodename`.

### Interface Discovery
*   **`getifaddrs()`**: Discovers active network interfaces and returns a linked list of `struct ifaddrs` containing their names, flags, and address structures.
*   **`freeifaddrs()`**: Safely releases the dynamically allocated list returned by `getifaddrs()`.
*   **`inet_ntop()`**: Formats binary address structures (both IPv4 `struct sockaddr_in` and IPv6 `struct sockaddr_in6`) into human-readable strings.
*   **`socket(AF_INET, SOCK_DGRAM, 0)`**: Establishes a temporary socket file descriptor to issue interface queries.
*   **`ioctl(sock, SIOCGIFMTU, ...)`**: Retrieves the Maximum Transmission Unit (MTU) configuration.
*   **`ioctl(sock, SIOCGIFHWADDR, ...)`**: Retrieves the interface's hardware/MAC address configuration.
*   **`close()`**: Closes socket descriptors to prevent file descriptor leaks.

### Routing & Default Gateway
*   **`/proc/net/route`**: Read via POSIX standard calls (`open`, `read`, `close`). This file exposes the active IPv4 routing table:
    *   `Destination`, `Gateway`, and `Mask` are represented as 8-character little-endian hexadecimal values.
    *   `Flags` represent routing rules (e.g., `0x0001` for `RTF_UP`, `0x0002` for `RTF_GATEWAY`).

---

## 3. Gateway Detection Algorithm
A route entry represents the **Default Gateway** if and only if:
1. The **Destination** field is `00000000` (which corresponds to `0.0.0.0`, representing default traffic routes).
2. The **Gateway** field is non-zero (indicating that packets are forwarded to a specific gateway router instead of being routed locally).
3. The entry is active (typically marked with `RTF_UP | RTF_GATEWAY` flags).

The hex strings are parsed and transformed back into standard IPv4 dotted-decimal representations using bit-shifted little-endian memory mapping.

---

## 4. Execution Flow

The Network Manager submenu dispatches selections as follows:

```
+-------------------------------------------------------------+
|                     network_mgr_run()                       |
|                     (Interactive Menu)                      |
+-------------------------------------------------------------+
                               |
            +------------------+------------------+------------------+
            |                                     |                  |
            v                                     v                  v
   [Option 1: Host Info]                 [Option 2: Interfaces]     [Option 3: Routing]
            |                                     |                  |
            v                                     v                  v
  network_show_host_info()              network_list_interfaces()  network_show_routes()
            |                                     |                  |
            +--> gethostname()                    +--> getifaddrs()  +--> open("/proc/net/route")
            +--> getaddrinfo() (AI_CANONNAME)     +--> ioctl()       +--> parse_routing_table()
            +--> uname() (nodename)               +--> inet_ntop()   +--> print output
```

---

## 5. Example Outputs

### Host Network Identity (Option 1)
```text
=== Host Network Identity ===
Hostname:           sandbox.example.com
Short Hostname:     sandbox
FQDN:               sandbox.example.com
Domain Name:        example.com
Kernel Node Name:   sandbox
=============================
```

### Interface Information (Option 2)
```text
Discovered Network Interfaces:
--------------------------------------------------
Interface Name: lo
Type:           Loopback
Status:         UP
MTU:            65536
MAC Address:    N/A
IPv4 Address:   127.0.0.1
IPv6 Address:   ::1
--------------------------------------------------
Interface Name: eth0
Type:           Ethernet
Status:         UP
MTU:            1500
MAC Address:    00:16:3e:4a:2c:12
IPv4 Address:   10.0.2.15
IPv6 Address:   fe80::216:3eff:fe4a:2c12
--------------------------------------------------
```

### Routing Information (Option 3)
```text
Default Gateway: 10.0.2.2

Interface  Destination      Gateway          Netmask          Flags 
----------------------------------------------------------------------
eth0       10.0.2.0         0.0.0.0          255.255.255.0    U     
eth0       0.0.0.0          10.0.2.2         0.0.0.0          UG    
----------------------------------------------------------------------
```

---

## 6. Error Handling
*   **Volatile File Reads**: Reading `/proc/net/route` checks for `ENOENT` (file missing) and `EACCES` (permission denied) errors to notify users cleanly.
*   **Host Resolution Errors**: If DNS or host translation via `getaddrinfo` fails, the module records a warning/error in the system logs, falls back to hostname values dynamically, and returns successfully without halting execution.
*   **Socket I/O Robustness**: Temporary sockets for `ioctl` queries are closed immediately after operations to prevent resource exhaustion.
*   **Memory Allocations**: All structures use stack allocation buffers rather than dynamic heaps, ensuring zero leaks.

---

## 7. Future Work (Unimplemented Stubs)
The following features are stubbed with `"Coming in a future sprint."`:
*   **Option 4: DNS Information**: Resolving server entries in `/etc/resolv.conf`.
*   **Option 5: Traffic Statistics**: Monitoring bandwidth in `/proc/net/dev`.
