# Đề tài 23: Linux System Manager

Dự án **Linux System Manager** là một ứng dụng quản lý hệ thống Linux hợp nhất, được thiết kế dưới dạng tài liệu tham khảo giảng dạy cho học phần Lập trình hệ thống Linux (Ubuntu).

Dự án này tích hợp các kiến thức cốt lõi về lập trình Linux vào một ứng dụng C dạng menu dòng lệnh (Terminal TUI) kết hợp với các mô-đun hạt nhân và kịch bản shell.

---

## 1. Cấu trúc Thư mục Dự án

* **`app/`**: Chứa luồng chính của chương trình (`main.c`), menu (`menu.c`) và mô-đun ghi log đồng bộ (`logger.c`).
* **`include/`**: Chứa các file tiêu đề chung (`modules.h`, `menu.h`, `logger.h`, `ipc_demo.h`).
* **`config/`**: Lưu trữ các file cấu hình hệ thống (`sysmgr.conf`).
* **`logs/`**: Lưu trữ log hoạt động hệ thống (`system.log`).
* **`modules/`**: Thư mục chứa các mô-đun chức năng ở không gian người dùng:
  - `file/`: Quản lý file sử dụng hệ thống System Calls trực tiếp (`open`, `read`, `write`, `stat`, `chmod`).
  - `process/`: Mô phỏng và kiểm soát tiến trình (`fork`, `exec`, `wait`, Zombie/Orphan demonstrations).
  - `signal/`: Quản lý bắt và xử lý tín hiệu bằng `sigaction`.
  - `socket/`: Chương trình nhắn tin mạng đa luồng (TCP Socket chat).
  - `network/`: Thu thập số liệu mạng và địa chỉ từ `/proc/net`, `/sys/class/net`.
  - `scheduler/`: Quản lý lập lịch cron jobs hệ thống.
  - `package/`: Giao diện bao đóng cài đặt phần mềm sử dụng `apt` thông qua tiến trình con.
  - `systeminfo/`: Hiển thị thông tin hệ thống thông qua `sysinfo()` và `uname()`.
  - `logviewer/`: Đọc và tìm kiếm nhanh file log hệ thống.
  - `ipc/`: (Tùy chọn) Các chương trình minh họa cơ chế IPC độc lập (Shared Memory, Semaphore, Named Pipe, Message Queue).
* **`kernel/`**: Mô-đun hạt nhân đơn giản `system_monitor.ko` sử dụng các tham số để cấu hình thông qua `printk`.
* **`shell/`**: Các kịch bản shell tự động hóa công việc quản trị (`backup.sh`, `update.sh`, `monitor.sh`, `permission.sh`).
* **`logs/`**: Chứa logs hoạt động.

---

## 2. Hướng dẫn Biên dịch và Cài đặt

### Cài đặt thư viện hạt nhân cần thiết
Trước khi biên dịch mô-đun hạt nhân, hãy chắc chắn rằng hệ thống đã cài đặt đầy đủ Linux headers:
```bash
sudo apt-get update
sudo apt-get install -y linux-headers-generic
```

### Biên dịch ứng dụng chính (Userspace application)
Để biên dịch ứng dụng chính và mô-đun thư viện:
```bash
make
```
Lệnh này sẽ tạo ra file thực thi `sysmgr` ở thư mục gốc của dự án.

### Chạy ứng dụng chính
Chạy ứng dụng quản trị hệ thống bằng lệnh:
```bash
./sysmgr
```

### Biên dịch mô-đun hạt nhân (Kernel Module)
Biên dịch mô-đun hạt nhân bằng cách chạy lệnh:
```bash
make kernel-module
```
Kết quả biên dịch sẽ tạo ra file mô-đun hạt nhân `kernel/system_monitor/system_monitor.ko`.

### Biên dịch các chương trình minh họa IPC (Optional IPC Demos)
Để biên dịch các chương trình minh họa IPC độc lập:
```bash
make ipc-demos
```
Các file thực thi sẽ được xuất ra thư mục `modules/ipc/bin/` (ví dụ: `modules/ipc/bin/shm_demo`).

---

## 3. Cách chạy và kiểm tra các Mô-đun

### Tải mô-đun hạt nhân
Tải mô-đun hạt nhân bằng cách chạy:
```bash
sudo insmod kernel/system_monitor/system_monitor.ko log_level=2 target_process="sysmgr"
```

Xem kết quả dmesg của hạt nhân:
```bash
dmesg | tail -n 20
```

Gỡ bỏ mô-đun hạt nhân:
```bash
sudo rmmod system_monitor
```

### Chạy các kịch bản Shell
Các file kịch bản trong thư mục `shell/` phải được phân quyền thực thi trước khi chạy:
```bash
chmod +x shell/*.sh
```
Ví dụ chạy kịch bản sao lưu:
```bash
./shell/backup.sh ./include ./backups
```