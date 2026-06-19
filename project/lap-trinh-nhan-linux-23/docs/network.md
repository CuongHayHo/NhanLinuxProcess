# Network Information Module - Sprint 2 Developer Documentation

## 1. Purpose
The purpose of the Network Information Module is to discover, retrieve, and display hostname, network interface adapters, MAC addresses, IPv4/IPv6 addresses, default gateway, DNS servers, detailed routing tables, and real-time interface packets/bytes statistics, demonstrating standard Linux filesystem-exposed state and networking APIs.

## 2. Linux APIs and Data Sources Used
* **`getifaddrs()`**: Queries active network adapters and returns a linked list of `struct ifaddrs`.
* **`inet_ntop()`**: Translates network address structures to standard presentation format strings.
* **`socket()`**: Opens a transient domain socket (`AF_INET`, `SOCK_DGRAM`) specifically to run adapter control queries.
* **`ioctl()`**: Invokes control functions (specifically the `SIOCGIFHWADDR` opcode) to fetch hardware MAC addresses.
* **`close()`**: Cleanly releases open sockets and files.
* **`gethostname()`**: Retrieves the host computer's hostname.
* **`/proc/net/route`**: System path read via low-level `open()`, `read()`, `close()` to parse default gateway and active system routing tables.
* **`/etc/resolv.conf`**: System path read via `open()`, `read()`, `close()` to parse nameserver entries for active DNS servers.
* **`/proc/net/dev`**: System path read via `open()`, `read()`, `close()` to parse real-time RX/TX statistics (bytes and packets) for all adapters.

## 3. Public APIs

### `void network_mgr_run(void)`
* **Purpose**: Dispatcher that performs a consolidated run of hostname, gateway, DNS, interface list, routing tables, and interface statistics.
* **Parameters**: None.
* **Return value**: None.

### `int network_list_interfaces(void)`
* **Purpose**: Gathers and prints a table of all interface names, status flags (UP/DOWN), MAC addresses, IPv4, and IPv6 addresses.
* **Parameters**: None.
* **Return value**: `0` on success, `-1` on failure.

### `int network_show_hostname(void)`
* **Purpose**: Queries and displays the system hostname.
* **Parameters**: None.
* **Return value**: `0` on success, `-1` on failure.

### `int network_show_ip(void)`
* **Purpose**: Lists IP addresses associated with all interfaces.
* **Parameters**: None.
* **Return value**: `0` on success, `-1` on failure.

### `int network_show_mac(void)`
* **Purpose**: Queries and lists MAC addresses of all interfaces.
* **Parameters**: None.
* **Return value**: `0` on success, `-1` on failure.

### `int network_show_gateway(void)`
* **Purpose**: Queries and prints the default system gateway.
* **Parameters**: None.
* **Return value**: `0` on success, `-1` on failure.

### `int network_show_dns(void)`
* **Purpose**: Parses configured nameservers and prints them.
* **Parameters**: None.
* **Return value**: `0` on success, `-1` on failure.

### `int network_show_routes(void)`
* **Purpose**: Parses and prints the complete routing table.
* **Parameters**: None.
* **Return value**: `0` on success, `-1` on failure.

### `int network_show_statistics(void)`
* **Purpose**: Parses and prints real-time interface traffic bytes/packets.
* **Parameters**: None.
* **Return value**: `0` on success, `-1` on failure.

---

## 4. Internal Design & Parsers

### File Parsing Helper (`read_entire_file`)
To eliminate code duplication and prevent buffer truncation on systems with large routing tables or many virtual network interfaces, a dynamic file reader is used:
* **Mechanism**: Opens a target file path in read-only mode using POSIX `open()`. Allocates an initial 4KB heap buffer and reads chunk-by-chunk using `read()`. If the file size exceeds the current buffer, it doubles the buffer size using `realloc()` dynamically.
* **Cleanup**: Guarantees that files are closed and buffers are freed upon completion or error detection to prevent resource leaks.

### Gateway Implementation & Route Parser
Reads `/proc/net/route` using standard low-level POSIX filesystem system calls.
* **Gateway**: Scans the file line-by-line, splitting fields by white space. A line is recognized as the default gateway if the `Destination` field is `"00000000"` (representing `0.0.0.0`) and the `Gateway` field is not `"00000000"`.
* **Route Table**: Converts little-endian hex representations of IP addresses to native standard dotted-decimal IPs using `sscanf()` and `inet_ntop()`. Flags (such as `U` for UP and `G` for GATEWAY) are decoded based on standard routing bitmasks (`RTF_UP = 0x0001`, `RTF_GATEWAY = 0x0002`).

### DNS Parser
Reads `/etc/resolv.conf` line-by-line. It ignores comment lines starting with `#` or `;` (and strips leading whitespaces) and parses lines beginning with the key `nameserver`, extracting the subsequent IP address to display the DNS servers.

### Statistics Parser
Reads `/proc/net/dev` and skips the first two lines of text headers. For each subsequent line:
1. Locates the colon (`:`) separator mapping the interface name.
2. Extracts statistics using `sscanf()` to parse the Receive (columns 1-2 for RX bytes and packets) and Transmit sections (columns 9-10 for TX bytes and packets).

## 5. Execution Flow
```
network_mgr_run()
  ├── Log: "Network information requested"
  ├── network_show_hostname()      ──> Calls gethostname()
  ├── network_show_gateway()       ──> Parses /proc/net/route
  ├── network_show_dns()           ──> Parses /etc/resolv.conf
  ├── network_list_interfaces()    ──> Gathers interface properties
  ├── network_show_routes()        ──> Renders full route tables
  └── network_show_statistics()    ──> Parses /proc/net/dev
```

## 6. Error Handling
* **File Missing / Permissions**: If opening system route, dev, or DNS configuration files fails (e.g., `ENOENT` or `EACCES`), the module logs the specific issue (e.g. `Missing file` or `Permission denied`) and prints `Unknown` or `None configured`, proceeding cleanly without crash.
* **Socket / ioctl failure**: Any failure when retrieving the MAC address is logged, falling back to `N/A`.
* **Parser Robustness**: Malformed configurations or invalid route entries are skipped, ensuring the module never halts prematurely.

## 7. Test Cases
Test cases are located in `tests/network_test.c`:
1. Call `network_show_hostname()`.
2. Call `network_show_ip()`.
3. Call `network_show_mac()`.
4. Call `network_show_gateway()`.
5. Call `network_show_dns()`.
6. Call `network_show_routes()`.
7. Call `network_show_statistics()`.
8. Consolidated run with `network_mgr_run()`.

## 8. Example Output
```
=== Network Information Panel ===
Hostname:          sandbox
Default Gateway:   172.17.0.1
DNS Servers:       127.0.0.11

Interface  Status   MAC Address        IPv4 Address     IPv6 Address    
------------------------------------------------------------------------------------
lo         UP       00:00:00:00:00:00  127.0.0.1        ::1             
eth0       UP       02:42:ac:11:00:02  172.17.0.2       fe80::42:acff:fe11:2
------------------------------------------------------------------------------------

Routing Table:
Interface  Destination      Gateway          Genmask          Flags  Metric
------------------------------------------------------------------------------------
eth0       0.0.0.0          172.17.0.1       0.0.0.0          UG     0     
eth0       172.17.0.0       0.0.0.0          255.255.0.0      U      0     

Interface Statistics:
Interface  RX Bytes         RX Packets   TX Bytes         TX Packets  
------------------------------------------------------------------------------------
lo         12345            100          12345            100         
eth0       9876543          2500         8765432          2400        
------------------------------------------------------------------------------------
=================================
```

## 9. Future Improvements
* **Advanced Statistics**: Traffic rate calculation (e.g. KB/s rx/tx rate tracking) using polling timers.
* **Live Socket Diagnostics**: Monitor active socket connections and listening ports from `/proc/net/tcp` and `/proc/net/udp`.
