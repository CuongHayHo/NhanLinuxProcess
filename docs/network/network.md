# HƯỚNG DẪN KỸ THUẬT VÀ ĐẶC TẢ CHI TIẾT PHÂN HỆ NETWORK MANAGER (/network)

Tài liệu này cung cấp các đặc tả thiết kế, kiến trúc mạng, luồng gọi hệ thống, phân tích mã nguồn và tích hợp API hệ thống liên quan đến phân hệ **Network Manager (`/network`)** trong hệ thống **Linux System Manager (sysmgr)**.

---

## 1. TỔNG QUAN PHÂN HỆ (MODULE OVERVIEW)
Phân hệ Network Manager (`/network`) cung cấp giao diện quản trị và chẩn đoán cấu hình mạng cho Linux. Phân hệ kết hợp việc sử dụng trực tiếp các cuộc gọi hệ thống POSIX mạng của C với việc gọi các lệnh CLI hệ thống Linux thông tin mạng (`ip link`, `ip route`, `ping`, `ss`, `netstat`) thông qua cơ chế nạp tiến trình con an toàn.

Các tính năng chính:
- **Liệt kê và chẩn đoán Card mạng:** Liệt kê tên thiết bị, xem trạng thái hoạt động (UP/DOWN), địa chỉ IPv4/IPv6, địa chỉ vật lý MAC, chỉ số truyền tải cực đại MTU và thống kê lưu lượng gói truyền nhận (`rx_packets`, `tx_packets`) qua sysfs.
- **Cấu hình Card mạng:** Bật/tắt đường truyền vật lý (UP/DOWN) và mô phỏng (Learning Mode) các lệnh thay đổi cấu hình mạng.
- **Xem bảng định tuyến:** Xem bảng định tuyến (Routing table) và xác định cổng kết nối mặc định (Default Gateway).
- **Kiểm tra kết nối và phân giải tên miền:** Chạy tiện ích Ping kiểm tra độ trễ mạng và phân giải DNS tên miền (DNS lookup) thành các địa chỉ IPv4/IPv6.
- **Thống kê Socket hoạt động:** Liệt kê các tiến trình đang lắng nghe cổng mạng (Socket statistics).

---

## 2. CÂY THƯ MỤC PHÂN HỆ (FILE TREE)
Dưới đây là các tệp nguồn liên quan trực tiếp đến phân hệ Network Manager:

1. **[include/network_mgr.h](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/include/network_mgr.h)**:
   - *Vai trò:* Tệp tiêu đề khai báo các nguyên mẫu hàm công khai cho REPL và TUI gọi.
2. **[modules/network/network_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/modules/network/network_mgr.c)**:
   - *Vai trò:* Tệp nguồn chính chứa toàn bộ logic truy vấn thông tin mạng bằng API POSIX C, đọc sysfs, phân tích `/proc/net/route` và thực thi vỏ lệnh điều khiển mạng.
3. **[tests/network_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/tests/network_test.c)**:
   - *Vai trò:* Chứa các kịch bản kiểm thử tự động, kiểm tra tính đúng đắn của các hàm phân tích địa chỉ IP, đọc cấu hình và lấy thông tin card mạng.

---

## 3. MỐI LIÊN HỆ VỚI CÁC TÀI LIỆU LÝ THUYẾT NHÂN (REFERENCE PDFs)

Phân hệ Network Manager áp dụng trực tiếp các khái niệm trong:

### A. Tài liệu `Phan 2. T2.L2-P9_Network.pdf` (Lập trình Mạng)
* **Khái niệm tương thích:** Phân tầng mô hình giao thức TCP/IP, cấu trúc gói tin mạng, cấu tạo socket buffer mức nhân (`sk_buff`), các lệnh cấu hình card mạng và bảng định tuyến của hệ điều hành.
* **Cách dự án áp dụng:**
  - Thể hiện trực tiếp các thông số MTU, địa chỉ IP (IPv4/IPv6) và địa chỉ MAC.
  - Đọc cấu trúc trạng thái bảng định tuyến mạng mức nhân tại tệp tin đặc biệt **`/proc/net/route`**, phân tích các cờ trạng thái định tuyến (Flags: `U` - Up, `G` - Gateway, `H` - Host).

### B. Tài liệu `Phan 2. T2.L2-P5_Socket.pdf` (Lập trình Sockets)
* **Khái niệm tương thích:** Cơ chế phân giải tên miền (DNS Lookup), lập trình Socket UDP/TCP và định dạng cấu trúc địa chỉ mạng (`sockaddr_in`, `sockaddr_in6`).
* **Cách dự án áp dụng:**
  - Sử dụng hàm gọi hệ thống **`getaddrinfo()`** để tra cứu máy chủ và chuyển đổi chuỗi tên miền dạng văn bản sang địa chỉ số IP.
  - Sử dụng hàm **`inet_ntop()`** để dịch các cấu trúc nhị phân địa chỉ IPv4/IPv6 mức nhân sang dạng chuỗi biểu diễn văn bản.

---

## 4. CHI TIẾT CÁC HÀM THÀNH VIÊN (FOR EVERY FUNCTION)

### 4.1. Hàm `network_list_interfaces_clean`
* **Nguyên mẫu (Prototype):** `int network_list_interfaces_clean(void)`
* **Tệp nguồn / Header:** `network_mgr.c` / `network_mgr.h`
* **Mục đích:** Liệt kê danh sách tên của toàn bộ các card mạng có trên máy chủ mà không bị lặp tên.
* **Luồng thực thi:**
  - Gọi hàm hệ thống **`getifaddrs()`** để lấy danh sách liên kết các cấu trúc `ifaddrs`.
  - Chạy một vòng lặp `for` duyệt qua danh sách liên kết.
  - Sử dụng một mảng lưu danh sách các tên card mạng đã được in (`processed`) để lọc bỏ tên trùng (vì getifaddrs trả về nhiều bản ghi cho cùng một card mạng nếu nó có cả IPv4, IPv6, hoặc các cấu hình phụ).
  - Trả về `0` nếu thành công, hoặc `-1` nếu lỗi. Gọi `freeifaddrs()` để giải phóng bộ nhớ.

### 4.2. Hàm `network_interface_info`
* **Nguyên mẫu:** `int network_interface_info(void)`
* **Tệp nguồn / Header:** `network_mgr.c` / `network_mgr.h`
* **Mục đích:** Truy vấn và hiển thị chi tiết mọi thông số cấu hình và lưu lượng truyền tải của một card mạng cụ thể.
* **Luồng thực thi:**
  - Yêu cầu người dùng nhập tên giao diện mạng cần chẩn đoán.
  - Duyệt danh sách liên kết từ `getifaddrs()` để tìm giao diện khớp tên.
  - Kiểm tra cờ trạng thái `ifa_flags` với cờ `IFF_UP` để in trạng thái `UP` hoặc `DOWN`.
  - Duyệt danh sách để in toàn bộ IP tĩnh gán cho card mạng đó:
    - Nếu là `AF_INET`, ép kiểu cấu trúc sang `struct sockaddr_in` và dịch qua `inet_ntop`.
    - Nếu là `AF_INET6`, ép kiểu sang `struct sockaddr_in6`.
  - Khởi tạo socket tạm thời `socket(AF_INET, SOCK_DGRAM, 0)` và sử dụng **`ioctl`** với tham số **`SIOCGIFMTU`** để lấy MTU của interface.
  - Sử dụng `ioctl` với tham số **`SIOCGIFHWADDR`** để đọc Hardware Address (địa chỉ vật lý MAC).
  - Gọi hàm trợ giúp `read_sysfs_stat` để đọc các tham số đếm gói truyền nhận trong thư mục `/sys/class/net/<interface>/statistics/rx_packets` và `tx_packets`.

### 4.3. Hàm `network_configure_interface_learning`
* **Nguyên mẫu:** `void network_configure_interface_learning(void)`
* **Tệp nguồn / Header:** `network_mgr.c` / `network_mgr.h`
* **Mục đích:** Hiển thị bài học hướng dẫn cách cấu hình gán IP tĩnh và bật card mạng bằng vỏ lệnh Linux tiêu chuẩn.
* **Chi tiết:** Giúp lập trình viên học các lệnh thiết lập mạng mức nhân:
  - `ip addr add <ip_address>/<mask_bits> dev <iface>`
  - `ip link set <iface> up`

### 4.4. Hàm `network_bring_interface_state`
* **Nguyên mẫu:** `void network_bring_interface_state(const char* state)`
* **Tệp nguồn / Header:** `network_mgr.c` / `network_mgr.h`
* **Mục đích:** Kích hoạt (UP) hoặc vô hiệu hóa (DOWN) đường truyền vật lý của card mạng.
* **Chi tiết:** Người dùng có thể chọn chế độ Learning Mode (chỉ hiện lệnh) hoặc Apply Mode (chạy thực tế). Nếu chọn chạy thực tế, hàm gọi `run_ip_link_command` để tạo tiến trình thực thi lệnh `ip link set <iface> <up/down>`.

### 5.5. Hàm `network_show_routes_ip`
* **Nguyên mẫu:** `int network_show_routes_ip(void)`
* **Tệp nguồn / Header:** `network_mgr.c` / `network_mgr.h`
* **Mục đích:** Xem bảng định tuyến IP hiện tại của hệ thống.
* **Chi tiết:** Tạo tiến trình chạy lệnh `ip route`. Nếu lệnh này thất bại (hệ thống thiếu lệnh `ip`), tự động kích hoạt cơ chế dự phòng gọi hàm `network_show_routes()` để tự phân tích thủ công tệp tin trạng thái định tuyến của nhân `/proc/net/route`.

### 5.6. Hàm `network_dns_lookup`
* **Nguyên mẫu:** `int network_dns_lookup(void)`
* **Tệp nguồn / Header:** `network_mgr.c` / `network_mgr.h`
* **Mục đích:** Tra cứu địa chỉ IP tương ứng với tên miền bằng DNS Server.
* **Chi tiết:**
  - Đặt cấu hình bộ lọc `hints` với `ai_family = AF_UNSPEC` (để lấy cả IPv4 lẫn IPv6).
  - Gọi hàm hệ thống **`getaddrinfo(hostname, NULL, &hints, &res)`**.
  - Nếu thành công, duyệt danh sách liên kết trả về và dịch địa chỉ nhị phân sang chuỗi số thông qua `inet_ntop`. Giải phóng bộ nhớ kết quả bằng **`freeaddrinfo(res)`**.

### 5.7. Hàm `network_show_routes` (Cơ chế phân tích /proc/net/route)
* **Nguyên mẫu:** `int network_show_routes(void)`
* **Mục đích:** Phân tích trạng thái định tuyến mức nhân không cần gọi chương trình ngoài.
* **Chi tiết:**
  - Mở tệp tin nhân `/proc/net/route` bằng cuộc gọi `open` chế độ `O_RDONLY`.
  - Đọc nội dung tệp vào vùng đệm bằng hàm hệ thống `read()`.
  - Gọi hàm phân tích `parse_routing_table` để bóc tách thông tin.

### 5.8. Hàm `parse_routing_table`
* **Nguyên mẫu:** `int parse_routing_table(const char* buf)`
* **Mục đích:** Bóc tách dữ liệu thô dạng bảng trong nhân Linux sang thông tin định tuyến trực quan.
* **Chi tiết:**
  - Phân tích chuỗi dòng bằng cách duyệt ký tự xuống dòng `\n`.
  - Với mỗi dòng, sử dụng hàm `sscanf` định dạng: `%31s %31s %31s %x %*d %*d %*d %31s` để trích xuất: Tên card mạng, Địa chỉ đích (hex), Gateway (hex), cờ Flags và Netmask (hex).
  - Gọi hàm tĩnh `hex_to_ip` để dịch các chuỗi Hex IP (ví dụ: `0101A8C0` ➔ `192.168.1.1`) sang dạng số thập phân chấm tiêu chuẩn.
  - Phân tích cờ Flags: `0x0001` là `U` (Up), `0x0002` là `G` (Gateway), `0x0004` là `H` (Host).
  - In bảng định tuyến định dạng đẹp mắt ra màn hình.

---

## 5. CÁC API HỆ THỐNG POSIX SỬ DỤNG

* **`getifaddrs()` / `freeifaddrs()`**: Khởi tạo và giải phóng cấu trúc liên kết chứa thông số tất cả card mạng.
* **`socket(AF_INET, SOCK_DGRAM, 0)`**: Mở một socket UDP tạm thời để truyền tham số truy vấn ioctl mà không cần kết nối mạng.
* **`ioctl(sock, cmd, &ifr)`**: Thực hiện kiểm soát thiết bị I/O hệ thống:
  - **`SIOCGIFMTU`**: Lấy kích thước MTU.
  - **`SIOCGIFHWADDR`**: Lấy địa chỉ vật lý MAC.
* **`inet_ntop()`**: Dịch cấu trúc nhị phân IP sang chuỗi ký tự hiển thị.
* **`getaddrinfo()` / `freeaddrinfo()`**: Phân giải tên miền DNS ra địa chỉ IP.
* **`gethostname()`**: Lấy tên Hostname định danh của máy chủ hiện tại.
* **`uname()`**: Lấy thông tin cấu trúc nhân và phiên bản OS của máy.

---

## 6. CẤU TRÚC DỮ LIỆU MẠNG LẬP TRÌNH NHÂN (NETWORK STRUCTURES)

* **`struct ifaddrs`**: Lưu thông số giao diện mạng (địa chỉ, subnet mask, broadcast, cờ trạng thái).
* **`struct ifreq`**: Cấu trúc tham số truyền qua cuộc gọi hệ thống `ioctl` để truy vấn hoặc thay đổi thuộc tính card mạng cụ thể thông qua trường `ifr_name`.
* **`struct sockaddr_in` / `struct sockaddr_in6`**: Cấu trúc lưu trữ thông số socket và địa chỉ IP chuẩn tương ứng với giao thức IPv4 và IPv6.
* **`struct addrinfo`**: Cấu trúc chứa thông tin địa chỉ máy chủ hỗ trợ phân giải DNS đa năng.
* **`struct utsname`**: Cấu trúc chứa thông số hệ điều hành nhân Linux (sysname, nodename, release, version, machine).

---

## 7. KIỂM THỬ PHÂN HỆ (TESTS)
* **Tệp kiểm thử:** [tests/network_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/tests/network_test.c)
* **Quy trình xác thực tự động:**
  - Kiểm tra tính đúng đắn của cơ chế bóc tách tệp `/proc/net/route` bằng cách giả lập một bộ đệm chứa bảng định tuyến tĩnh và kiểm tra kết quả giải mã của hàm `parse_routing_table`.
  - Gọi liệt kê card mạng để kiểm tra tính tương thích của API `getifaddrs()`.
  - Kiểm tra hàm lấy thông tin host (`network_show_host_info()`).

---

## 8. ĐÁP ỨNG YÊU CẦU BÀI TẬP (ASSIGNMENT TRACEABILITY)
Phân hệ đáp ứng hoàn hảo yêu cầu về **"Linux Programming (Process & POSIX Network control APIs)"**:
* **POSIX Network control APIs:** Đã thực hiện thông qua việc sử dụng trực tiếp các API chuẩn `getifaddrs`, `socket`, `ioctl`, `inet_ntop`, và `getaddrinfo` để kiểm soát các thông số mạng mà không phụ thuộc vào vỏ lệnh.
* **Process control:** Gọi các lệnh ping, ss, ip link một cách an toàn bằng luồng tiến trình con Fork-Exec đồng bộ.

---

## 9. VÍ DỤ SỬ DỤNG VÀ KẾT QUẢ ĐẦU RA MONG ĐỢI (EXAMPLES)

### 9.1. Xem chi tiết thông tin card mạng `lo`
```bash
sysmgr/network > /interfaces

--- Available Network Interfaces ---
lo
eth0
------------------------------------
Enter interface name: lo

========================================
Interface Name: lo
State:          UP
IPv4:           127.0.0.1
IPv6:           ::1
MAC Address:    00:00:00:00:00:00
MTU:            65536
RX packets:     10283
TX packets:     10283
========================================
```

### 9.2. Phân giải tên miền DNS
```bash
sysmgr/network > /dns
Enter hostname to resolve (e.g. google.com): google.com

Resolved addresses for google.com:
- IPv4: 172.217.161.78
- IPv6: 2404:6800:4003:c03::64
```
