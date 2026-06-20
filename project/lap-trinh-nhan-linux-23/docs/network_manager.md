# Linux Network Interface Manager

This document describes the design, implementation, execution flow, error handling, and Linux system data sources for the **Linux Network Interface Manager** (formerly Network Manager) module.

---

## 1. Purpose
The Linux Network Interface Manager module serves as an interactive diagnostic utility tailored for an undergraduate systems programming context. It allows administrators to query interface configuration, verify packet routing, simulate configuration commands, change interface states, execute network probes, and inspect open socket states.

---

## 2. Menu Structure
The manager presents the following console-driven TUI:
```text
========================================
Network Manager
========================================
1. List Network Interfaces
2. Interface Information
3. Configure Interface (Learning Mode)
4. Bring Interface UP
5. Bring Interface DOWN
6. Show Routing Table
7. Ping Host
8. DNS Lookup
9. Socket Statistics
0. Return
========================================
```

---

## 3. Features & Data Sources

### 1. List Network Interfaces (Read Only)
*   **Purpose**: Display a clean list of all available interface identifiers.
*   **Implementation**: Discovered dynamically using the `getifaddrs()` function to parse active interfaces. Duplicates are removed using unique string matching in userspace.
*   **Example Output**:
    ```text
    lo
    eth0
    wlan0
    ```

### 2. Interface Information (Read Only)
*   **Purpose**: Inspect details of a selected interface.
*   **Implementation**:
    *   State (UP/DOWN) is verified using the device flags returned by `getifaddrs()` (`IFF_UP`).
    *   IPv4 & IPv6 addresses are fetched by walking the `struct sockaddr` allocations matching the interface name.
    *   MAC Address and MTU configurations are retrieved using standard Linux `ioctl()` requests (`SIOCGIFHWADDR` and `SIOCGIFMTU`).
    *   RX and TX packet statistics are parsed by opening `/sys/class/net/<iface>/statistics/rx_packets` and `tx_packets` volatile kernel stats files.
*   **Output Metrics**:
    *   Interface Name
    *   State (UP/DOWN)
    *   IPv4 (if assigned)
    *   IPv6 (if assigned)
    *   MAC Address
    *   MTU
    *   RX packets
    *   TX packets

### 3. Configure Interface (Learning Mode Only)
*   **Purpose**: Teach students how to assign static IPv4 configurations using the standard iproute2 suite.
*   **Security Constraint**: NEVER modifies the host system.
*   **Behavior**: Generates the exact shell command string (e.g. `ip addr add 192.168.1.100/24 dev eth0` and `ip link set eth0 up`) and explains every command parameter step-by-step.

### 4 & 5. Bring Interface UP / DOWN (System Modification / Learning Mode)
*   **Purpose**: Enable or disable network interfaces.
*   **Interactive TUI Prompt**: Asks the administrator to choose between:
    1.  *Learning Mode*: Displays the command string: `ip link set <iface> up` or `ip link set <iface> down`.
    2.  *Apply for real*: Executes the mutation using a `fork()` + `execvp()` process invocation of `ip`.
*   **Error Handling**: If the process returns non-zero exit codes (like `1` or `255`), the manager detects permission failures, catches the exception, and informs the user that root privileges (sudo) are required.

### 6. Show Routing Table (Read Only)
*   **Purpose**: Display active network pathways.
*   **Implementation**: Spawns `ip route` directly to display table rules. If execution fails, it falls back to parsing `/proc/net/route` dynamically.

### 7. Ping Host (Read Only)
*   **Purpose**: Verify remote ICMP host reachability.
*   **Implementation**: Prompts the user for a destination address (IP or domain name) and packet count, then executes `/bin/ping -c <count> <host>` cleanly.

### 8. DNS Lookup (Read Only)
*   **Purpose**: Resolve hostnames to IP addresses.
*   **Implementation**: Uses the standard POSIX resolver API `getaddrinfo()` with `AF_UNSPEC` to fetch both `AF_INET` (IPv4) and `AF_INET6` (IPv6) records, walking the linked list of addresses.

### 9. Socket Statistics (Read Only)
*   **Purpose**: Display current active network sockets.
*   **Implementation**: Invokes `ss -tulnp` (socket statistics) to output protocol, state, local/remote address, and process identifiers. Displays a tip that root privileges (sudo) are required to query process identifiers (PIDs).

---

## 4. Safety & Sandboxing Boundaries

| Feature | Classification | Host Side-Effects | Elevated Privileges Required |
|---------|----------------|-------------------|------------------------------|
| List Interfaces | Read Only | None | No |
| Interface Info | Read Only | None | No |
| Configure Interface | Learning Mode | None | No |
| Bring UP/DOWN (Learning) | Learning Mode | None | No |
| Bring UP/DOWN (Apply) | System Modification | Mutates Interface Link State | Yes (Requires `sudo`) |
| Show Routing | Read Only | None | No |
| Ping Host | Read Only | None | No |
| DNS Lookup | Read Only | None | No |
| Socket Statistics | Read Only | None | Yes (only for PID mapping) |
