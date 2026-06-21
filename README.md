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
6.  **Shell Manager**: Thực thi lệnh, chạy script `/bin/bash`, quản lý biến môi trường, tự động hóa tác vụ (Automation/Cron) và cấu hình thời gian (Time Configuration: cấu hình thủ công qua clock_settime/CAP_SYS_TIME, và tự động đồng bộ thời gian qua Internet bằng cơ chế HTTP fallback an toàn).
    *   **Đồng bộ thời gian tự phục hồi (Auto-recovery Time Synchronization)**: Dự án tích hợp kịch bản tự phục hồi `shell/autosync.sh` giải quyết triệt để vấn đề "quả trứng và con gà" giữa DNSSEC và NTP khi thời gian hệ thống bị lệch sâu về quá khứ (ví dụ năm 2004). Hệ thống tự lấy mốc thời gian gần đúng bằng cách truy vấn HTTP trực tiếp qua IP (bỏ qua DNSSEC/SSL) rồi mới kích hoạt `chronyc`/`ntpdate` để đồng bộ chính xác, đảm bảo hoạt động an toàn tuyệt đối.
7.  **Kernel Module**: Giao tiếp kernel space qua `/proc/sysmgr` và tích hợp các tài liệu sơ đồ mạng (luồng gói tin Network Stack, cấu trúc bộ đệm `sk_buff`, và cơ chế điều phối NAPI).

---

## 4. Hướng dẫn Build (Build Instructions)

Build toàn bộ dự án và các chương trình phụ trợ (sysmgr, socket binaries):
```bash
make clean && make
```

Build các module kiểm thử riêng lẻ:
```bash
make test-file       # Build kiểm thử File Manager
make test-process    # Build kiểm thử Process Manager
make test-network    # Build kiểm thử Network Manager
make test-package    # Build kiểm thử Package Manager
make test-socket     # Build kiểm thử Socket Manager
make test-kernel     # Build kiểm thử Kernel Manager
make test-shell      # Build kiểm thử Shell Manager
make test-logger     # Build kiểm thử POSIX Logger
```

---

## 5. Hướng dẫn Chạy Test (Test Instructions)

Chạy các bộ kiểm thử tự động tương ứng:
```bash
./tests/file_test
./tests/process_test
./tests/network_test
./tests/package_test
./tests/socket_test
./tests/kernel_test
./tests/shell_test
./tests/logger_test
```

---

## 6. Giao diện Người dùng (User Interface Modes)

Dự án hỗ trợ 3 chế độ giao diện khác nhau nhằm đem lại trải nghiệm quản trị Linux hiện đại và tối ưu:

### A. Chế độ TUI Interactive Menu (Arrow-key Selection)
Khi ứng dụng khởi chạy ở chế độ cổ điển (`./sysmgr --classic`), toàn bộ Menu chính, các Submenu của từng Module, và các câu hỏi xác nhận (ví dụ: xác nhận tải kernel module) đều tự động nhận diện và sử dụng phím mũi tên `UP`/`DOWN` để di chuyển và `ENTER` để chọn mục thay vì gõ số thủ công.

*Nhãn giải thích hành động:* Khi người dùng chọn một tác vụ yêu cầu tham số bổ sung, hệ thống sẽ tự động hiển thị mô tả trực quan và ý nghĩa của hành động bằng màu xanh Cyan (`[Action Info] <mô tả>`) ngay trước lời nhắc nhập dữ liệu.

### B. Chế độ CLI REPL hiện đại (Mặc định)
Khi chạy lệnh `./sysmgr` không có tham số, chương trình sẽ khởi chạy chế độ Command Line REPL chuyên nghiệp:
*   **Gợi ý lệnh (Auto-suggestions & Autocomplete):** Gõ `/` để mở bảng gợi ý phím nóng và lệnh. Bấm phím `TAB` hoặc phím mũi tên sang phải để tự động hoàn thành lệnh. Bảng gợi ý hỗ trợ hiển thị tối đa lên đến 16 gợi ý đồng thời.
*   **Termios raw mode:** Điều hướng và sửa lỗi văn bản trực tiếp bằng phím mũi tên và phím `BACKSPACE`.
*   **Lịch sử lệnh (History command):** Sử dụng mũi tên lên/xuống để duyệt lại lịch sử lệnh đã gõ.

### C. Chế độ non-interactive CLI (Tương thích Scripting)
Khi chạy dạng `./sysmgr [Module] [Command] [Args]`, ứng dụng sẽ tự động vô hiệu hóa chế độ tương tác (`is_interactive = 0`), chạy lệnh trực tiếp và in kết quả ra thiết bị đầu ra chuẩn (stdout) để phục vụ cho viết script tự động hóa hoặc tích hợp giao diện GUI bên thứ ba.