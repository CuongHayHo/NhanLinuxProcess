# Linux Kernel Module and Networking Fundamentals

This document describes the lifecycle, APIs, compilation, loading, and verification processes of a Linux Kernel Module (LKM), alongside educational documentation of the Linux Network Stack, `sk_buff` packet structures, and NAPI interrupt mitigation.

---

## 1. Kernel Module Lifecycle

A loadable kernel module (LKM) undergoes several state transitions managed by the Linux kernel:

```
    [Source Code: system_monitor.c]
                  | (make)
                  v
       [Object Binary LKM: .ko]
                  |
                  v (insmod / module_init)
             [LKM loaded] ---> Executes initialization logic & printk()
                  |
                  v (Running in kernel space)
          [Kernel execution]
                  |
                  v (rmmod / module_exit)
            [LKM unloaded] ---> Executes cleanup logic, releases resources & printk()
```

1.  **Compilation**: C code is compiled against target kernel headers using Kbuild to produce a `.ko` (kernel object) file.
2.  **Loading**: The `insmod` tool loads the module into kernel space, triggering the entry function registered via `module_init()`.
3.  **Active**: The module runs with ring 0 privileges inside kernel space.
4.  **Unloading**: The `rmmod` tool unloads the module, triggering the cleanup function registered via `module_exit()`.
5.  **Cleaned**: The module memory and symbols are removed from the running kernel.

---

## 2. Essential Kernel APIs and Macros

Unlike user-space programs that use `main()` and link with glibc, kernel modules rely on direct kernel sub-systems and macros:

### `module_init()`
*   **Purpose**: Designates the entry function executed when the module is inserted.
*   **Prototype**:
    ```c
    static int __init my_init_function(void);
    module_init(my_init_function);
    ```
*   **Notes**: The `__init` macro tells the kernel that the function is only used during initialization, freeing its memory after execution.

### `module_exit()`
*   **Purpose**: Designates the cleanup function executed when the module is removed.
*   **Prototype**:
    ```c
    static void __exit my_exit_function(void);
    module_exit(my_exit_function);
    ```
*   **Notes**: The `__exit` macro tells the kernel that this code can be omitted if the kernel does not support module unloading.

### `printk()`
*   **Purpose**: Kernel-space logging (writes to the kernel circular buffer).
*   **Usage**:
    ```c
    printk(KERN_INFO "system_monitor: LKM loaded successfully.\n");
    ```
*   **Notes**: Standard library functions (like `printf`) are unavailable in kernel space because LKMs do not link with glibc. Log levels (like `KERN_INFO`, `KERN_WARNING`, `KERN_ERR`) control verbosity and routing.

---

## 3. The `/proc` Filesystem and `seq_file` API

To enable user-space programs to query kernel-level LKM details without invoking system calls or custom device hooks, this module implements a read-only `/proc` entry under `/proc/sysmgr`.

### Execution Flow: User Space $\rightarrow$ Kernel Space

```
+---------------------------------------+
|  User Space: cat /proc/sysmgr         |
+---------------------------------------+
                   | (Triggers read() syscall)
                   v
+---------------------------------------+
|  VFS (Virtual File System)            |
+---------------------------------------+
                   | (Dispatches to proc_ops)
                   v
+---------------------------------------+
|  seq_read() / sysmgr_proc_show()      |
|  (Kernel space / LKM logic)           |
+---------------------------------------+
                   | (Retrieves Uptime, metadata, stats)
                   v
+---------------------------------------+
|  seq_printf() formats data            |
+---------------------------------------+
                   | (Returns payload back to VFS)
                   v
+---------------------------------------+
|  Terminal prints formatted data       |
+---------------------------------------+
```

1.  **Virtual File System (VFS)**: When a user-space utility runs `cat /proc/sysmgr`, the OS translates it into a standard `read()` system call directed to the virtual `/proc` directory.
2.  **`seq_file` Interface**: In the kernel, raw file read requests can be difficult to manage because output might span multiple blocks. The `seq_file` API provides helper routines to sequentially format text blocks.
    *   **`single_open()`**: Opens the file descriptor and binds a format helper (`sysmgr_proc_show()`).
    *   **`seq_read()`**: Handles sequential page copying of data to the user space buffer.
    *   **`seq_printf()`**: Standard print formatter writing safe seq strings directly into the seq kernel buffers.
3.  **`proc_ops` Registration**: In modern kernels (Linux 5.6+), we register these operations using `struct proc_ops` to hook the VFS entry.
4.  **Creation and Cleanup**:
    *   `proc_create("sysmgr", 0444, NULL, &sysmgr_proc_ops)` creates the file descriptor inside `/proc` upon module load.
    *   `remove_proc_entry("sysmgr", NULL)` cleans it up during module unload.

---

## 4. Submenu and Educational Demonstrations

To match the networking focus of the Linux programming curriculum, the Kernel Module menu has been refactored into a classroom-ready demonstration layout:

```text
========================================
Kernel Module
========================================
1. Show Module Information
2. Load Module
3. Unload Module
4. Show Network Stack Overview
5. Show sk_buff Overview
6. Show NAPI Overview
0. Return
========================================
```

### 1. Show Module Information
*   **Verification**: Checks if `/proc/sysmgr` is readable.
*   **Output**: Displays the file contents (Module Name, Version, Kernel Version, load time).
*   **Fallback**: If the module is not loaded (file unavailable), prompts the user if they would like to compile and load the module automatically.

### 2. Load Module
*   **Compilation**: Checks if `system_monitor.ko` exists in `kernel/system_monitor/`. If missing, compiles it automatically using `make`.
*   **Insertion**: Loads the module into the running kernel using `sudo insmod system_monitor.ko`.
*   **Verification**: Confirms `/proc/sysmgr` becomes available.

### 3. Unload Module
*   **Removal**: Runs `sudo rmmod system_monitor` using `fork()` + `execvp()`.
*   **Verification**: Checks that `/proc/sysmgr` is removed cleanly.

### 4. Show Network Stack Overview
Displays the packet traversal pipeline taught in Linux kernel programming, describing both transmission (TX) and reception (RX) pathways:
```text
   Application
        ↓
     Socket       (BSD Socket Interface, sock_create(), sys_socket())
        ↓
    TCP/UDP       (Transport Layer, tcp_v4_rcv(), tcp_sendmsg())
        ↓
  Network Stack   (Network/IP Layer, ip_rcv(), ip_output())
        ↓
     sk_buff      (Socket Buffer - packet metadata & structure)
        ↓
     Driver       (Net Device Driver, dev_queue_xmit(), ndo_start_xmit())
        ↓
      NIC         (Network Interface Card - Physical Layer / Hardware)
```
*   **Application**: Invokes `write()`, `send()`, or `sendto()` syscalls on a file descriptor.
*   **Socket**: Translates file I/O to network socket calls, initializing socket queues.
*   **TCP/UDP**: transport layer protocol engine. Handles checksums, sequencing, flow control, and state machines.
*   **Network Stack**: Network/IP layer. Computes routing tables, handles IP fragments, checksums, and packet filtering (Netfilter).
*   **sk_buff**: Allocates kernel descriptors that track packet metadata across memory.
*   **Driver**: Pushes packet descriptors to the device ring buffer and issues TX interrupts.
*   **NIC**: Converts memory frames into serial streams (electrical/optical) onto physical media.

### 5. Show sk_buff Overview
Details the core data buffer descriptor structure in the Linux kernel (`include/linux/skbuff.h`):
*   **Purpose**: Tracks packets without copying payloads as they traverse network layers. Layers prepend/strip headers by shifting internal pointer boundaries.
*   **Crucial Fields**:
    *   `next`, `prev`: Queue links for buffering.
    *   `dev`: Points to managing `net_device`.
    *   `len`, `data_len`: Total bytes and fragment sizes.
    *   `head`, `data`, `tail`, `end`: Boundary pointers tracking allocated heap memory boundaries.
    *   `transport_header`, `network_header`, `mac_header`: Offsets to protocols headers.
*   **API Lifecycle Operations**:
    *   `alloc_skb()`: Allocates buffer headers.
    *   `skb_reserve()`: Reserves room for packet headers.
    *   `skb_put()`: Extends the data area down (moves `tail`).
    *   `skb_push()`: Extends the data area up (moves `data` left to write headers).
    *   `skb_pull()`: Shinks the data area from the start (moves `data` right to strip headers).
    *   `kfree_skb()`: Releases the buffer back to kernel pools.

### 6. Show NAPI Overview
Explains Linux's **New API (NAPI)** hybrid interrupt-mitigation framework:
*   **The Livelock Problem**: At 10Gbps+ speeds, triggering hardware interrupts for every packet saturates the CPU, leaving zero cycles for user applications (Receive Livelock).
*   **Hybrid Mitigation**:
    1.  *First Packet*: Arrives and triggers a hardware interrupt.
    2.  *Disable*: Driver disables NIC interrupts and schedules polling.
    3.  *Poll*: Kernel polls the driver (`poll()` method) processing packets in batches (budget size, e.g., 64).
    4.  *Re-enable*: Once the ring buffer is drained, polling is disabled and NIC interrupts are re-enabled.
*   **Benefits**: High throughput throughput stability, decreased context switching overhead, and early drop policies under congestion.
