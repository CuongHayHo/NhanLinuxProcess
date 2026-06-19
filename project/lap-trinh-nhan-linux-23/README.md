# NhanLinuxProcess

NhanLinuxProcess là một dự án C về lập trình hệ thống Linux, gồm ứng dụng dòng lệnh, các module userspace, ví dụ IPC, script shell và một kernel module đơn giản.

## Cấu trúc chính

- `app/`: chương trình chính (`main.c`, `menu.c`, `logger.c`)
- `modules/`: thư viện và các module chức năng như file, process, signal, socket, network, scheduler, package, systeminfo, logviewer
- `c/`: các chương trình C độc lập để demo file, process, network, socket
- `kernel/system_monitor/`: kernel module `system_monitor`
- `shell/`: các script hỗ trợ thao tác hệ thống
- `config/`: file cấu hình
- `tests/`: test C cho logger và file
- `logs/`: log runtime của ứng dụng

## Build

Chạy từ thư mục `project/lap-trinh-nhan-linux-23`:

```bash
make
```

Các target bổ sung:

```bash
make ipc-demos
make kernel-module
make test-logger
make test-file
```

## Kết quả build

- `make` tạo binary `sysmgr`
- `make ipc-demos` tạo các binary IPC trong `modules/ipc/bin/`
- `make kernel-module` tạo file `.ko` trong `kernel/system_monitor/`
- `make test-logger` và `make test-file` tạo binary test trong `tests/`

## Chạy

```bash
./sysmgr
```

Ví dụ với kernel module:

```bash
sudo insmod kernel/system_monitor/system_monitor.ko log_level=2 target_process="sysmgr"
dmesg | tail -n 20
sudo rmmod system_monitor
```

## Dọn dẹp

```bash
make clean
```