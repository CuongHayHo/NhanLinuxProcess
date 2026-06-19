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
1.  **File Manager**: Quản lý và duyệt thư mục hệ thống.
2.  **Process Manager**: Quản lý tiến trình (liệt kê, tìm kiếm, tín hiệu, nice value) và demo vòng đời (Fork, Exec, Wait, Zombie, Orphan, Daemon).
3.  **Network Manager**: Hiển thị card mạng (`getifaddrs`), bảng định tuyến `/proc/net/route` và tên máy tính.
4.  **Socket Manager**: Quản lý kết nối TCP/IP đa luồng (Multi-client TCP Server).
5.  **Package Manager**: Tự động nhận diện `rpm` / `dpkg` và truy vấn siêu dữ liệu của các gói cài đặt.
6.  **Shell Manager**: Thực thi lệnh, chạy script `/bin/bash`, quản lý biến môi trường, tự động hóa tác vụ (Automation/Cron) và cấu hình thời gian (Time Configuration: cấu hình thủ công qua clock_settime/CAP_SYS_TIME, đồng bộ thời gian qua Internet sử dụng chronyc/ntpdate, và cấu hình đồng bộ tự động NTP sử dụng timedatectl).
    *   **Cơ chế Đồng bộ Xác thực (Verified Synchronization)**: Khác biệt hoàn toàn so với việc chỉ gửi **yêu cầu (request)** đồng bộ thời gian (chỉ kiểm tra lệnh chạy thành công), ứng dụng sẽ đợi 2 giây và thực hiện kiểm tra độc lập qua trạng thái của `chronyc tracking` hoặc `timedatectl status` để đảm bảo đồng hồ hệ thống thực sự đã được đồng bộ chính xác trước khi báo thành công.
7.  **Kernel Module**: Giao tiếp và đọc thông tin từ kernel space qua `/proc/sysmgr`.

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