# NhanLinuxProcess (Linux System Manager)

NhanLinuxProcess là một ứng dụng quản lý hệ thống Linux viết bằng ngôn ngữ C, giới thiệu các API lập trình hệ thống Linux và POSIX chuẩn. Dự án bao gồm các module quản trị hệ thống ở tầng userspace, cơ chế liên lạc liên tiến trình (IPC), shell script, và một kernel module đơn giản.

---

## 1. Khái niệm Chế độ Học tập (Learning Mode)
Dự án chạy hoàn toàn trong **Learning Mode** (Chế độ Chỉ đọc). Mục tiêu là cung cấp một môi trường an toàn tuyệt đối để tìm hiểu cách thức hệ thống Linux hoạt động:
*   Mọi tác vụ truy vấn dữ liệu (liệt kê tiến trình, tra cứu gói cài đặt, cấu hình thời gian) đều sử dụng các luồng đọc trực tiếp từ kernel hoặc cơ sở dữ liệu hệ thống.
*   Các thao tác ghi hoặc thay đổi trạng thái (cài đặt/gỡ bỏ phần mềm, cấu hình clock/NTP, đổi múi giờ) bị chặn và hiển thị ở chế độ Xem trước (Preview).

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
[FileMgr]      [ProcMgr]      [NetMgr] [PkgMgr]    [TimeMgr]      [SysInfo]
(O_RDONLY)     (/proc)     (getifaddrs)  (rpm/dpkg)   (sysinfo)     (uname)
```

Kiến trúc bao gồm:
*   **Chương trình Core (`app/`)**: Quản lý vòng lặp Menu TUI và định tuyến lựa chọn.
*   **Hệ thống Modules (`modules/`)**: Các chức năng tác vụ độc lập được đóng gói vào thư viện tĩnh `libmodules.a`.
*   **Ví dụ Độc lập (`examples/`)**: Các demo nhỏ gọn để kiểm tra và học tập các system call gốc của Linux.

---

## 3. Tổng quan các Module (Module Overview)
*   **Logger**: Hệ thống ghi log đồng bộ dùng system call POSIX (`open`, `write`, `close`).
*   **File Manager (Stub)**: Thao tác file mức hệ thống.
*   **Process Manager**: Quản lý tiến trình (liệt kê, tìm kiếm, tín hiệu, độ ưu tiên) và các demo vòng đời (Fork, Exec, Wait, Zombie, Orphan, Daemon).
*   **Network Manager**: Hiển thị card mạng (`getifaddrs`), bảng định tuyến `/proc/net/route` và tên máy tính.
*   **Package Manager**: Tự động nhận diện `rpm` / `dpkg` và truy vấn siêu dữ liệu (metadata) của các gói cài đặt.
*   **Time Manager**: Truy vấn ngày, giờ địa phương, UTC, Unix epoch, múi giờ và thời gian chạy hệ thống (`sysinfo`).

---

## 4. Hướng dẫn Build (Build Instructions)

Build toàn bộ dự án:
```bash
make clean && make
```

Build các module kiểm thử riêng lẻ:
```bash
make test-package  # Build test package manager
make test-time     # Build test time manager
```

---

## 5. Hướng dẫn Chạy Test (Test Instructions)

Chạy bộ kiểm thử tự động của Package Manager:
```bash
./tests/package_test
```

Chạy bộ kiểm thử tự động của Time Manager:
```bash
./tests/time_test
```

---

## 6. Ảnh chụp màn hình (Screenshots)

### Giao diện Menu chính (Main Menu TUI)
```text
========================================
      Linux System Manager (v0.7.0-rc1)
========================================
1. File Manager
2. Process Manager
3. Signal Manager
4. Network Manager
5. Socket Chat
6. Package Manager
7. Scheduler
8. System Information
9. Log Viewer
10. Kernel Module
11. Time Manager
0. Exit
----------------------------------------
Select option: 
```

### Giao diện Time Manager
```text
========================================
              Time Manager
========================================
1. Show Current Time
2. Time Zone (Future)
3. Set Time (Preview)
4. NTP Status (Future)
0. Return
========================================
```