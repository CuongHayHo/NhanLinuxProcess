# NhanLinuxProcess (Linux System Manager)

NhanLinuxProcess là một ứng dụng quản lý hệ thống Linux viết bằng ngôn ngữ C, giới thiệu các API lập trình hệ thống Linux và POSIX chuẩn. Dự án bao gồm các module quản trị hệ thống ở tầng userspace, shell script, và một kernel module đơn giản.

---

## 1. Khái niệm Chế độ Học tập (Learning Mode)
Dự án chạy hoàn toàn trong **Learning Mode** (Chế độ Chỉ đọc). Mục tiêu là cung cấp một môi trường an toàn tuyệt đối để tìm hiểu cách thức hệ thống Linux hoạt động:
*   Mọi tác vụ truy vấn dữ liệu (liệt kê tiến trình, card mạng, gói cài đặt) đều sử dụng các luồng đọc trực tiếp từ kernel hoặc cơ sở dữ liệu hệ thống.
*   Các thao tác ghi hoặc thay đổi trạng thái bị chặn và hiển thị ở chế độ Mô phỏng / Xem trước (Preview).

---

## 2. Kiến trúc Dự án (Project Architecture)

```
                       +---------------------------+
                       |       TUI Main Menu       |
                       |       (app/main.c)        |
                       +---------------------------+
                                     |
    +--------------+--------------+--+--+--------------+--------------+
    |              |              |     |              |              |
    v              v              v     v              v              v
[FileMgr]      [ProcMgr]      [NetMgr] [SockMgr]   [PkgMgr]       [ShellMgr]
(O_RDONLY)     (/proc)     (getifaddrs) (TCP Echo) (rpm/dpkg)    (fork/exec)
```

Kiến trúc bao gồm:
*   **Chương trình Core (`app/`)**: Quản lý vòng lặp Menu TUI và định tuyến lựa chọn.
*   **Hệ thống Modules (`modules/`)**: Các chức năng tác vụ độc lập được đóng gói vào thư viện tĩnh `libmodules.a`.

---

## 3. Tổng quan các Module (Module Overview)
Dự án được cấu trúc chặt chẽ quanh 7 module cốt lõi theo đúng yêu cầu bài tập lớn:
1.  **File Manager**: Được thiết kế đúng theo yêu cầu "Lập trình shell để quản lý file". Toàn bộ logic quản lý file (tạo, đọc, ghi, xóa, sao chép, di chuyển, phân quyền, lưu trữ, tìm kiếm) được thực hiện trong kịch bản shell `shell/file.sh`. Ứng dụng C chỉ đóng vai trò là launcher TUI để hiển thị menu và gọi shell script bằng fork-exec.
2.  **Process Manager**: Quản lý tiến trình (liệt kê, tìm kiếm, tín hiệu, nice value) và demo vòng đời (Fork, Exec, Wait, Zombie, Orphan, Daemon).
3.  **Network Manager**: Quản lý card mạng Linux (`getifaddrs()`, chi tiết trạng thái, IPv4/IPv6/MAC/MTU, và gói tin RX/TX, cấu hình mô phỏng Learning Mode, bật/tắt UP/DOWN, hiển thị bảng định tuyến `ip route`, ping kiểm thử, phân giải tên miền `getaddrinfo()`, và hiển thị thống kê socket `ss`).
4.  **Socket Manager**: Quản lý kết nối TCP/IP đa luồng (Multi-client TCP Server) có cơ chế ngắt kết nối an toàn với đầy đủ `shutdown()` và `close()`.
5.  **Package Manager**: Được thiết kế đúng theo yêu cầu bài tập lớn "Lập trình shell để cài đặt/gỡ bỏ các chương trình tự động". Toàn bộ logic quản lý gói (tìm kiếm, thông tin, cài đặt, gỡ bỏ, demo vòng đời) được thực hiện trong kịch bản shell `shell/program.sh`. Ứng dụng C chỉ đóng vai trò là launcher TUI để hiển thị menu và gọi shell script bằng fork-exec.
6.  **Shell Manager**: Thực thi lệnh, chạy script `/bin/bash`, quản lý biến môi trường, tự động hóa tác vụ (Automation/Cron) và cấu hình thời gian (Time Configuration: cấu hình thủ công qua clock_settime/CAP_SYS_TIME, đồng bộ thời gian qua Internet sử dụng chronyc/ntpdate, và cấu hình đồng bộ tự động NTP sử dụng timedatectl).
    *   **Cơ chế Đồng bộ Xác thực (Verified Synchronization)**: Khác biệt hoàn toàn so với việc chỉ gửi **yêu cầu (request)** đồng bộ thời gian (chỉ kiểm tra lệnh chạy thành công), ứng dụng sẽ đợi 2 giây và thực hiện kiểm tra độc lập qua trạng thái của `chronyc tracking` hoặc `timedatectl status` để đảm bảo đồng hồ hệ thống thực sự đã được đồng bộ chính xác trước khi báo thành công.
7.  **Kernel Module**: Giao tiếp kernel space qua `/proc/sysmgr` và tích hợp các tài liệu sơ đồ mạng (luồng gói tin Network Stack, cấu trúc bộ đệm `sk_buff`, và cơ chế điều phối NAPI).

---

## 4. Hướng dẫn Build (Build Instructions)

Build toàn bộ dự án:
```bash
make clean && make
```

Build các module kiểm thử riêng lẻ:
```bash
make test-package    # Build test package manager
make test-shell      # Build test shell manager
make test-time_sync  # Build test time synchronization
```

---

## 5. Hướng dẫn Chạy Test (Test Instructions)

Chạy bộ kiểm thử tự động của Package Manager:
```bash
./tests/package_test
```

Chạy bộ kiểm thử tự động của Shell Manager:
```bash
./tests/shell_test
```

Chạy bộ kiểm thử đồng bộ thời gian Internet:
```bash
./tests/time_sync_test
```

---

## 6. Ảnh chụp màn hình (Screenshots)

### Giao diện Menu chính (Main Menu TUI)
```text
========================================
      Linux System Manager (v0.8.0)
========================================
1. File Manager
2. Process Manager
3. Network Manager
4. Socket Manager
5. Package Manager
6. Shell Manager
7. Kernel Module
0. Exit
----------------------------------------
Select option: 
```

### Giao diện Menu Quản lý Kernel Module (Kernel Module Submenu)
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
Select option: 
```