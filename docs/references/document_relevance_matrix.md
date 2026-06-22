# BẢN ĐỒ LIÊN KẾT TÀI LIỆU VÀ PHÂN HỆ MÃ NGUỒN (DOCUMENT RELEVANCE MATRIX)

Bản mô tả chi tiết này phân tích mối quan hệ giữa 9 tài liệu slide lý thuyết nằm trong thư mục [docs/references](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/docs/references) với 9 phân hệ chức năng (modules) trong mã nguồn dự án **Linux System Manager (sysmgr)**.

---

## 1. BẢNG MA TRẬN TỔNG QUAN (% Ứng dụng)

Dưới đây là tỷ lệ phần trăm ứng dụng lý thuyết từ các tài liệu slide vào việc thiết kế và viết mã nguồn cho từng chức năng trong `sysmgr`:

| Phân hệ chức năng trong `sysmgr` | P1_File.pdf | P1_Process.pdf (VFS) | P2_Signal.pdf | P3_Semaphore.pdf (Threads/Mutex) | P5_Socket.pdf | P6_SharedMemory.pdf | P7_VirtualMemory.pdf | P8_Device.pdf | P9_Network.pdf | Thực tế / Tiện ích hệ thống |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **`/file`** (File Manager) | **80%** | **20%** | 0% | 0% | 0% | 0% | 0% | 0% | 0% | 0% |
| **`/process`** (Process Manager) | 10% | 10% | **40%** | **20%** | 0% | 0% | 0% | 0% | 0% | **20%** |
| **`/network`** (Network Manager) | 10% | 0% | 0% | 0% | 20% | 0% | 0% | 0% | **70%** | 0% |
| **`/socket`** (Socket Manager) | 0% | 0% | 10% | **30%** | **60%** | 0% | 0% | 0% | 0% | 0% |
| **`/package`** (Package Manager) | 30% | 0% | 10% | 0% | 0% | 0% | 0% | 0% | 0% | **60%** |
| **`/shell`** (Shell Manager) | 40% | 10% | 0% | 0% | 0% | 0% | 0% | 0% | 0% | **50%** |
| **`/cron`** (Cron Job Manager) | 50% | 0% | 0% | 0% | 0% | 0% | 0% | 0% | 0% | **50%** |
| **`/time`** (Time Manager) | 0% | 0% | 0% | 0% | 0% | 0% | 0% | 20% | 0% | **80%** |
| **`/kernel`** (Kernel Module) | 20% | **20%** | 0% | 0% | 0% | 0% | 0% | **30%** | **30%** | 0% |

> [!NOTE]
> - Các tài liệu **`Phan 2. T2.L2-P6_SharedMemory.pdf`** và **`Phan 2. T2.L2-P7_ViturlMemory.pdf`** hoàn toàn không được ứng dụng trong dự án (`0%` trên tất cả chức năng).
> - Nhãn **"Thực tế / Tiện ích hệ thống"** đại diện cho tỷ lệ các mã lệnh cấu hình thực tế mức quản trị Linux (như Cron daemon, NTP, dnf/apt) không phụ thuộc vào lý thuyết lập trình nhân.

---

## 2. CHI TIẾT ỨNG DỤNG CHO TỪNG PHÂN HỆ

### 2.1. Phân hệ `/file` (File Manager)
* **Tệp mã nguồn liên quan:** [modules/file/file_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/modules/file/file_mgr.c)
* **Tỷ lệ phân bổ và lý giải:**
  - **80% từ `Phan 2. T2.L2-P1_File.pdf`**: Chứa toàn bộ lý thuyết và cú pháp của các hàm hệ thống POSIX làm việc với tệp tin: `open()`, `close()`, `read()`, `write()`, `lseek()`, `stat()`, `chmod()`, `mkdir()`, và `readdir()`. Đây là những hàm trực tiếp tạo nên tính năng tạo, đọc, sửa, phân quyền, xem thuộc tính và duyệt thư mục trong file manager.
  - **20% từ `Phan 2. T2.L2-P1_Process.pdf` (VFS)**: Lý giải cấu trúc trừu tượng mức nhân như `struct file`, `inode`, `dentry`, và `superblock` giải thích cách hạt nhân lưu trữ siêu dữ liệu (metadata) của tệp.

### 2.2. Phân hệ `/process` (Process Manager)
* **Tệp mã nguồn liên quan:** [modules/process/](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/modules/process/) (bao gồm `process_mgr.c` và các thư mục con demo `fork_demo.c`, `exec_demo.c`, `wait_demo.c`, `daemon_demo.c`).
* **Tỷ lệ phân bổ và lý giải:**
  - **40% từ `Phan 2. T2.L2-P2_Signal.pdf`**: Áp dụng trực tiếp vào tính năng gửi tín hiệu điều khiển và kết liễu tiến trình (`/process kill`) sử dụng hàm hệ thống `kill(pid, signo)` và các tín hiệu hệ thống như `SIGKILL (9)`, `SIGTERM (15)`.
  - **20% từ `Phan 2. T2.L2-P3_Semaphore.pdf`**: Sử dụng lý thuyết về lập lịch tiến trình và định nghĩa luồng điều khiển của CPU.
  - **10% từ `Phan 2. T2.L2-P1_Process.pdf` (VFS)**: Liên quan đến bảng mô tả tệp tin của tiến trình (`File table của process`), giải thích cách tiến trình quản lý các file descriptor `0`, `1`, `2`.
  - **10% từ `Phan 2. T2.L2-P1_File.pdf`**: Hỗ trợ việc duyệt thư mục đặc biệt `/proc` để liệt kê danh sách tiến trình đang hoạt động trong hệ thống.
  - **20% từ Lập trình thực tế**: Triển khai các kịch bản tiến trình Zombie, Orphan và tiến trình chạy ngầm Daemon.

### 2.3. Phân hệ `/network` (Network Manager)
* **Tệp mã nguồn liên quan:** [modules/network/network_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/modules/network/network_mgr.c)
* **Tỷ lệ phân bổ và lý giải:**
  - **70% từ `Phan 2. T2.L2-P9_Network.pdf`**: Áp dụng lý thuyết các tầng giao thức mạng TCP/IP, cấu trúc gói tin và socket buffer trong nhân để thực hiện cấu hình các thẻ mạng NIC (Network Interface Card).
  - **20% từ `Phan 2. T2.L2-P5_Socket.pdf`**: Cách phân giải tên miền (DNS resolution) và cấu trúc địa chỉ IP (`sockaddr_in`).
  - **10% từ `Phan 2. T2.L2-P1_File.pdf`**: Cách nhân Linux coi các giao diện mạng như các file để cấu hình thông qua cơ chế `ioctl`.

### 2.4. Phân hệ `/socket` (Socket Manager)
* **Tệp mã nguồn liên quan:** [modules/socket/](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/modules/socket/) (bao gồm `socket_server.c`, `socket_client.c`, `socket_multi_server.c`, và `socket_mgr.c`).
* **Tỷ lệ phân bổ và lý giải:**
  - **60% từ `Phan 2. T2.L2-P5_Socket.pdf`**: Cơ sở lý thuyết socket đơn giản truyền và nhận dữ liệu qua giao diện BSD Sockets API (`socket()`, `bind()`, `listen()`, `accept()`, `connect()`, `send()`, `recv()`).
  - **30% từ `Phan 2. T2.L2-P3_Semaphore.pdf`**: Cực kỳ quan trọng đối với máy chủ socket đa kết nối (`socket_multi_server.c`). Phân hệ sử dụng đa luồng sinh ra bởi **`pthread_create`** để quản lý song song nhiều kết nối client, đồng thời dùng khóa đồng bộ **`pthread_mutex_t`** để bảo vệ chống tranh chấp ghi luồng terminal output.
  - **10% từ `Phan 2. T2.L2-P2_Signal.pdf`**: Bắt tín hiệu ngắt đường truyền mạng `SIGPIPE` để xử lý khi kết nối socket bị ngắt đột ngột mà không làm sập tiến trình `sysmgr`.

### 2.5. Phân hệ `/package` (Package Manager)
* **Tệp mã nguồn liên quan:** [modules/package/package_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/modules/package/package_mgr.c) và kịch bản [shell/program.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/program.sh).
* **Tỷ lệ phân bổ và lý giải:**
  - **30% từ `Phan 2. T2.L2-P1_File.pdf`**: Kiểm tra sự tồn tại của trình quản lý gói (hàm `access()`) và thực thi kịch bản cài đặt (hàm `execvp()`).
  - **10% từ `Phan 2. T2.L2-P2_Signal.pdf`**: Đồng bộ tín hiệu của tiến trình con thực thi câu lệnh cài đặt.
  - **60% từ Tiện ích hệ thống**: Trình quản lý gói hệ thống (`apt-get`, `dnf`, `rpm`) là các chương trình mức phân phối Linux, không có tài liệu lý thuyết lập trình nhân tương ứng.

### 2.6. Phân hệ `/shell` (Shell Manager)
* **Tệp mã nguồn liên quan:** [modules/shell/shell_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/modules/shell/shell_mgr.c)
* **Tỷ lệ phân bổ và lý giải:**
  - **40% từ `Phan 2. T2.L2-P1_File.pdf`**: Đọc/ghi các biến môi trường cấu hình và thực thi tệp tin kịch bản.
  - **10% từ `Phan 2. T2.L2-P1_Process.pdf` (VFS)**: Lý thuyết định hướng luồng vào/ra tiêu chuẩn (`dup2` và chuyển hướng `/dev/null`).
  - **50% từ Tiện ích hệ thống**: Các lệnh tương tác trực tiếp với trình vỏ `/bin/sh` là lệnh không gian người dùng.

### 2.7. Phân hệ `/cron` (Cron Job Manager)
* **Tệp mã nguồn liên quan:** [cli/repl.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/cli/repl.c) (chức năng tạo và xóa cron).
* **Tỷ lệ phân bổ và lý giải:**
  - **50% từ `Phan 2. T2.L2-P1_File.pdf`**: Thao tác ghi luồng trực tiếp vào file cấu hình cron hệ thống (`/var/spool/cron/crontabs/` hoặc `/etc/cron.d/`) và thực hiện ghi nối tiếp (append) log đầu ra.
  - **50% từ Tiện ích hệ thống**: Cú pháp cron định thời và daemon điều hành cron.

### 2.8. Phân hệ `/time` (Time Manager)
* **Tệp mã nguồn liên quan:** [cli/repl.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/cli/repl.c) (chức năng xem/set clock).
* **Tỷ lệ phân bổ và lý giải:**
  - **20% từ `Phan 2. T2.L2-P8_Device.pdf`**: Ứng dụng gián tiếp khái niệm cấu hình hẹn giờ (Timer/Clock) của nhân.
  - **80% từ Tiện ích hệ thống**: Đồng bộ hóa NTP, daemon `chronyd` và các hàm thư viện thời gian chuẩn C.

### 2.9. Phân hệ `/kernel` (Kernel Module)
* **Tệp mã nguồn liên quan:** [modules/kernel/kernel_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/modules/kernel/kernel_mgr.c) và nhân LKM [kernel/system_monitor/system_monitor.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/kernel/system_monitor/system_monitor.c).
* **Tỷ lệ phân bổ và lý giải:**
  - **30% từ `Phan 2. T2.L2-P8_Device.pdf`**: Quy trình biên dịch nhân (`make`), tải module thông qua hàm gọi hệ thống tương ứng lệnh **`insmod`** và gỡ module bằng **`rmmod`**.
  - **30% từ `Phan 2. T2.L2-P9_Network.pdf`**: Chức năng `/kernel` trực tiếp kết xuất (render) và hiển thị các kiến thức cấu trúc mạng nâng cao mức nhân:
    - *Linux Network Stack Packet Flow*
    - *sk_buff struct overview*
    - *NAPI polling system overview*
  - **20% từ `Phan 2. T2.L2-P1_Process.pdf` (VFS)**: Lý thuyết kết nối nhân và không gian người dùng thông qua hệ thống tệp ảo **`/proc/sysmgr`** (sử dụng hàm `proc_create` và cấu trúc `seq_file`).
  - **20% từ `Phan 2. T2.L2-P1_File.pdf`**: Đọc luồng dữ liệu từ tệp tin đặc biệt `/proc/sysmgr` để hiển thị ra terminal của user-space.

---

## 3. CÁC TÀI LIỆU KHÔNG ĐƯỢC ỨNG DỤNG VÀ LÝ DO

1. **`Phan 2. T2.L2-P6_SharedMemory.pdf` (Shared Memory - 0%)**:
   - Cung cấp kiến thức về thiết lập bộ nhớ dùng chung IPC để hai tiến trình khác nhau trao đổi dữ liệu tốc độ cao.
   - *Lý do:* Hệ thống `sysmgr` thực hiện giao tiếp mạng bằng TCP/IP sockets hoặc gọi tiến trình trực tiếp, hoàn toàn không cần chia sẻ vùng nhớ chung RAM.

2. **`Phan 2. T2.L2-P7_ViturlMemory.pdf` (Virtual Memory - 0%)**:
   - Cung cấp kiến thức về cơ chế phân trang của MMU trong nhân CPU, bộ nhớ Cache và cách hạt nhân cấp phát trang RAM.
   - *Lý do:* Đây là lý thuyết phần cứng và quản lý mức thấp của OS, không trực tiếp hỗ trợ viết các ứng dụng quản lý trạng thái tiến trình hay mạng mức giám sát.
