# HƯỚNG DẪN KỸ THUẬT VÀ ĐẶC TẢ CHI TIẾT PHÂN HỆ TASK SCHEDULING & CRON (/cron)

Tài liệu này cung cấp tài liệu kỹ thuật, đặc tả thiết kế, phân tích mã nguồn chi tiết và kiểm thử của phân hệ **Task Scheduling & Cron (`/cron`)** trong dự án **Linux System Manager (sysmgr)**. Đây là tài liệu tham chiếu dành cho lập trình viên để duy trì, mở rộng và kiểm thử hệ thống.

---

## BẢNG MỤC LỤC
1. [TỔNG QUAN PHÂN HỆ (MODULE OVERVIEW)](#1-tổng-quan-phân-hệ-module-overview)
2. [CÂY THƯ MỤC PHÂN HỆ (FILE TREE & INVENTORY)](#2-cây-thư-mục-phân-hệ-file-tree--inventory)
3. [MỐI LIÊN HỆ VỚI CÁC TÀI LIỆU LÝ THUYẾT NHÂN (REFERENCE PDFs)](#3-mối-liên-hệ-với-các-tài-liệu-lý-thuyết-nhân-reference-pdfs)
4. [PHÂN TÍCH THIẾT KẾ VÀ KIẾN TRÚC HỆ THỐNG (SYSTEM ARCHITECTURE)](#4-phân-tích-thiết-kế-và-kiến-trúc-hệ-thống-system-architecture)
5. [CẤU TRÚC BIỂU THỨC CRON HỆ THỐNG (CRON SYNTAX)](#5-cấu-trúc-biểu-thức-cron-hệ-thống-cron-syntax)
6. [PHÂN TÍCH CHI TIẾT KỊCH BẢN BASH (SHELL SCRIPT ANALYSIS)](#6-phân-tích-chi-tiết-kịch-bản-bash-shell-script-analysis)
7. [ĐẶC TẢ CHI TIẾT CÁC HÀM C WRAPPERS (C FUNCTION SPECIFICATIONS)](#7-đặc-tả-chi-tiết-các-hàm-c-wrappers-c-function-specifications)
8. [TÍCH HỢP GIỮA C VÀ SHELL SCRIPT (C & SHELL INTEGRATION)](#8-tích-hợp-giữa-c-và-shell-script-c--shell-integration)
9. [CÁC LỆNH LINUX HỆ THỐNG SỬ DỤNG (LINUX COMMAND SPECS)](#9-các-lệnh-linux-hệ-thống-sử-dụng-linux-command-specs)
10. [THƯ VIỆN TIÊU CHUẨN SỬ DỤNG (STANDARD LIBRARIES)](#10-thư-viện-tiêu-chuẩn-sử-dụng-standard-libraries)
11. [AN NINH VÀ AN TOÀN LẬP LỊCH (SECURITY & SAFETY)](#11-an-ninh-và-an-toàn-lập-lịch-security--safety)
12. [HIỆU NĂNG VÀ TỐI ƯU HÓA (PERFORMANCE & OPTIMIZATION)](#12-hiệu-năng-và-tối-ưu-hóa-performance--optimization)
13. [BẢN ĐỒ TRUY XUẤT YÊU CẦU BÀI TẬP (ASSIGNMENT TRACEABILITY)](#13-bản-đồ-truy-xuất-yêu-cầu-bài-tập-assignment-traceability)
14. [BẢN ĐỒ TRUY XUẤT TÀI LIỆU THAM KHẢO (REFERENCE TRACEABILITY)](#14-bản-đồ-truy-xuất-tài-liệu-tham-khảo-reference-traceability)
15. [KIỂM THỬ VÀ CHẨN ĐOÁN LỖI (TEST SUITE & DIAGNOSTICS)](#15-kiểm-thử-và-chẩn-đoán-lỗi-test-suite--diagnostics)

---

## 1. TỔNG QUAN PHÂN HỆ (MODULE OVERVIEW)
Phân hệ **Task Scheduling & Cron (`/cron`)** cung cấp giao diện tích hợp cho phép quản lý vòng đời của các tác vụ lập lịch tự động (Scheduled Tasks) chạy dưới sự giám sát của daemon dịch vụ Cron hệ thống Linux. 

Phân hệ hỗ trợ đầy đủ các thao tác xem danh sách tác vụ đang lập lịch, tạo mới một tác vụ với biểu thức thời gian tùy chỉnh, và gỡ bỏ toàn bộ lịch biểu. Cơ chế cốt lõi của phân hệ là tận dụng pipeline và chuyển hướng dòng Standard Input để nạp trực tiếp lịch biểu mới vào tệp tin cấu hình người dùng (User Crontab file) thông qua lệnh hệ thống `crontab`, đảm bảo tính toàn vẹn dữ liệu và tránh ghi đè các cấu hình độc lập của người dùng.

---

## 2. CÂY THƯ MỤC PHÂN HỆ (FILE TREE & INVENTORY)
Danh sách toàn bộ các tập tin cấu thành phân hệ Task Scheduling bao gồm:

1. **[modules/shell/shell_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/modules/shell/shell_mgr.c)**:
   - *Vai trò:* Triển khai hàm C wrapper `shell_mgr_cron_execute` để phân tích dòng lệnh từ REPL và thực thi các cuộc gọi shell/POSIX tương ứng.
2. **[shell/crontab.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/crontab.sh)**:
   - *Vai trò:* Kịch bản shell cung cấp menu lựa chọn tạo, xóa, và liệt kê các tác vụ Cron trong môi trường vỏ shell độc lập.
3. **[shell/test_cron.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/test_cron.sh)**:
   - *Vai trò:* Kịch bản con ghi nhận thời gian nhịp tim (timestamp) kiểm thử sự thực thi của daemon dịch vụ Cron.
4. **[shell/monitor.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/monitor.sh)**:
   - *Vai trò:* Kịch bản con tương thích với Cron để ghi nhận thông tin phần cứng.
5. **[cli/repl.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/cli/repl.c)**:
   - *Vai trò:* Định tuyến lệnh nhập vào từ CLI REPL (ví dụ: `cron list`, `cron create`) tới hàm C `shell_mgr_cron_execute`.
6. **[tests/shell_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/tests/shell_test.c)**:
   - *Vai trò:* Kiểm tra sự tồn tại và quyền truy cập của kịch bản `shell/crontab.sh`.

---

## 3. MỐI LIÊN HỆ VỚI CÁC TÀI LIỆU LÝ THUYẾT NHÂN (REFERENCE PDFs)

Phân hệ Task Scheduling áp dụng trực tiếp kiến thức từ hai tài liệu lý thuyết cốt lõi của môn học:

### A. Tài liệu `Phan 2. T2.L2-P1_File.pdf` (Đọc ghi file và chuyển hướng)
* **Khái niệm áp dụng:** Cơ chế chuyển hướng luồng dữ liệu lỗi tiêu chuẩn (Standard Error Redirection) và ghép nối luồng dữ liệu (Pipelines).
* **Cách dự án áp dụng:**
  - Để lấy cấu hình Crontab hiện thời mà không in ra màn hình thông báo lỗi khi Crontab rỗng, phân hệ sử dụng: `crontab -l 2>/dev/null`. Số `2` đại diện cho File Descriptor của dòng Error.
  - Sử dụng pipeline và hàm ghép nối `cat` để truyền dữ liệu cấu hình cũ và mới làm Standard Input cho lệnh `crontab -`:
    ```bash
    crontab -l 2>/dev/null | { cat; echo "$cron_expression $script_path"; } | crontab -
    ```
    Đây là ứng dụng thực tế trực tiếp của việc quản lý bảng mô tả tệp tin (File descriptor table) của tiến trình Linux.

### B. Tài liệu `Phan 2. T2.L2-P1_Process.pdf` (Quản lý Tiến trình)
* **Khái niệm áp dụng:** Cơ chế tạo tiến trình con (`fork`), nạp mới chương trình vỏ shell (`exec`) và cha đợi con đồng bộ (`waitpid`).
* **Cách dự án áp dụng:**
  - Hàm C `shell_mgr_cron_execute` gọi hàm `shell_mgr_execute` để tạo tiến trình con, nạp chương trình `/bin/sh` thông dịch dòng lệnh cập nhật crontab. Cha gọi `waitpid` chặn luồng để chờ quá trình cập nhật crontab hoàn tất sạch sẽ.

---

## 4. PHÂN TÍCH THIẾT KẾ VÀ KIẾN TRÚC HỆ THỐNG (SYSTEM ARCHITECTURE)

### A. Quy trình tạo Tác vụ (Task Creation Workflow)
Quy trình thêm mới một lịch biểu Cron vào hệ thống được thiết kế như sau:

```mermaid
sequenceDiagram
    participant User
    participant REPL (C)
    participant Shell_Mgr (C)
    participant Crontab_Utility (OS)
    participant Cron_Daemon (OS)

    User->>REPL (C): Nhập lệnh (ví dụ: cron create /path/to/script "* * * * *")
    REPL (C)->>Shell_Mgr (C): shell_mgr_cron_execute()
    Shell_Mgr (C)->>Shell_Mgr (C): chmod +x /path/to/script
    Shell_Mgr (C)->>Crontab_Utility (OS): crontab -l 2>/dev/null | { cat; echo "* * * * * /path/to/script"; } | crontab -
    Crontab_Utility (OS)-->>Shell_Mgr (C): Cập nhật thành công (exit code = 0)
    Shell_Mgr (C)-->>User: In thông báo thành công
    Note over Cron_Daemon (OS): Tự động phát hiện thay đổi trong /var/spool/cron/
    Cron_Daemon (OS)->>Cron_Daemon (OS): Thực thi /path/to/script mỗi phút
```

### B. Vòng đời tác vụ (Task Lifecycle)
1. **Khởi tạo:** Người dùng thiết lập lịch biểu. Script mục tiêu được cấp quyền thực thi `chmod +x`.
2. **Nạp cấu hình:** Hệ điều hành ghi nhận lịch biểu vào thư mục spool cấu hình cá nhân `/var/spool/cron/crontabs/<username>`.
3. **Thực thi:** Daemon dịch vụ `cron` (hoặc `crond`) thức dậy mỗi phút quét thư mục spool, nạp lịch biểu và khởi chạy kịch bản ngầm. Đầu ra (stdout/stderr) của kịch bản được tự động gửi qua mail hệ thống hoặc chuyển hướng ghi vào tệp nhật ký `cron_test.log`.
4. **Hủy bỏ:** Lệnh `crontab -r` xóa sạch tệp cấu hình của người dùng, giải phóng hoàn toàn lịch biểu.

---

## 5. CẤU TRÚC BIỂU THỨC CRON HỆ THỐNG (CRON SYNTAX)

Biểu thức Cron tiêu chuẩn trong Linux bao gồm 5 trường phân cách bởi khoảng trắng:

```
.---------------- phút (0 - 59)
|  .------------- giờ (0 - 23)
|  |  .---------- ngày trong tháng (1 - 31)
|  |  |  .------- tháng (1 - 12)
|  |  |  |  .---- ngày trong tuần (0 - 6, Chủ nhật = 0 hoặc 7)
|  |  |  |  |
*  *  *  *  *  [đường_dẫn_tệp_kịch_bản]
```

### Các ký tự đặc biệt hỗ trợ:
*   `*` (Wildcard): Đại diện cho mọi giá trị hợp lệ của trường đó.
*   `,` (List): Phân tách danh sách các giá trị cụ thể. Ví dụ: `1,5 * * * *` chạy vào phút thứ 1 và phút thứ 5.
*   `-` (Range): Mô tả dải giá trị. Ví dụ: `0 9-17 * * *` chạy mỗi giờ trong khung giờ hành chính từ 9h đến 17h.
*   `/` (Interval): Mô tả bước nhảy thời gian. Ví dụ: `*/5 * * * *` chạy định kỳ mỗi 5 phút.

---

## 6. PHÂN TÍCH CHI TIẾT KỊCH BẢN BASH (SHELL SCRIPT ANALYSIS)

### 6.1. Kịch bản `shell/crontab.sh`
*   **Mục đích:** Cung cấp menu tương tác tạo, xóa và liệt kê các tác vụ Cron trong shell độc lập.
*   **Các hàm thành viên:**
    - `createCronJob()`:
      - Nhận đường dẫn tệp tin script từ bàn phím.
      - Gọi lệnh `[ -f "$filePath" ]` kiểm tra xem tệp tin script có tồn tại thực tế hay không. Nếu không, in thông báo lỗi và quay lại menu.
      - Nhập chuỗi biểu thức Cron.
      - Cấp quyền thực thi cho script: `chmod +x $filePath`.
      - Cập nhật crontab bằng pipeline:
        ```bash
        crontab -l | { cat; echo "$cronJobExpression $filePath"; } | crontab -
        ```
      - Trả về mã thoát `$?`. Nếu bằng `0`, thông báo tạo thành công.
    - `listCronJobs()`:
      - In ra danh sách crontab hiện tại của người dùng bằng cách gọi lệnh `crontab -l`.
    - `showMenu()`:
      - Vòng lặp rẽ nhánh bằng `case` thực thi các hàm trên hoặc gọi `crontab -r` (Xóa toàn bộ lịch biểu).

### 6.2. Kịch bản `shell/test_cron.sh`
*   **Mục đích:** Ghi nhận dấu ấn thời gian (timestamp heartbeat) xác thực việc daemon dịch vụ Cron đang thực thi kịch bản đúng định kỳ.
*   **Chi tiết luồng xử lý:**
    - Xác định thư mục làm việc của dự án:
      ```bash
      DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
      PROJECT_DIR="$( dirname "$DIR" )"
      ```
    - Tạo thư mục tạm thời `tmp/` nếu chưa tồn tại.
    - Ghi nối tiếp dòng log nhịp tim kèm ngày giờ hệ thống vào file đích:
      ```bash
      echo "[CRON TEST] Executed successfully at $(date)" >> "$PROJECT_DIR/tmp/cron_test.log"
      ```

---

## 7. ĐẶC TẢ CHI TIẾT CÁC HÀM C WRAPPERS (C FUNCTION SPECIFICATIONS)

### 7.1. Hàm `shell_mgr_cron_execute`
*   **Nguyên mẫu (Prototype):** `void shell_mgr_cron_execute(int argc, char** argv);`
*   **Tệp nguồn / Tiêu đề:** [shell_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/modules/shell/shell_mgr.c#L374) / [shell_mgr.h](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/include/shell_mgr.h#L31)
*   **Mục đích:** Hàm điều phối quản lý tác vụ Cron từ CLI hoặc TUI Menu.
*   **Chi tiết luồng xử lý:**
    - **Trường hợp có tham số dòng lệnh (`argc > 1`):**
      - Nếu `argv[1]` là `list` hoặc `show`: Thực thi lệnh `crontab -l` thông qua hàm `shell_mgr_execute`.
      - Nếu `argv[1]` là `delete` hoặc `clear`: Thực thi lệnh `crontab -r`.
      - Nếu `argv[1]` là `create`:
        - Phân tích cú pháp: Nếu thiếu tham số đường dẫn và biểu thức, chương trình gọi `linenoise` (ở chế độ interactive) hoặc `fgets` để thu thập dữ liệu nhập vào từ bàn phím.
        - Khởi dựng câu lệnh cấp quyền thực thi: `chmod +x <đường_dẫn>`.
        - Khởi dựng câu lệnh pipeline cập nhật crontab: `crontab -l 2>/dev/null | { cat; echo "<biểu_thức> <đường_dẫn>"; } | crontab -`.
        - Thực thi câu lệnh. Nếu trả về `0`, thông báo thành công; ngược lại báo lỗi thất bại.
    - **Trường hợp chạy tương tác (TUI Menu):**
      - Vẽ menu các tùy chọn và định tuyến tương ứng tới các cuộc gọi CLI tương đương.

---

## 8. TÍCH HỢP GIỮA C VÀ SHELL SCRIPT (C & SHELL INTEGRATION)
- Khi người dùng nhập lệnh CLI dạng `/cron create`, hàm `shell_mgr_cron_execute` được kích hoạt.
- Cấp quyền cho kịch bản đích bằng cách nạp `/bin/sh` chạy lệnh `chmod +x`.
- Thực thi cập nhật Crontab của OS thông qua cuộc gọi nạp `/bin/sh` xử lý chuỗi lệnh pipeline.
- Kết quả cập nhật Crontab được hệ điều hành lưu trữ lâu dài. Khi daemon `cron` của Linux chạy ngầm thức dậy, nó tự động nạp kịch bản `shell/test_cron.sh` hoặc `shell/monitor.sh` để chạy độc lập.

---

## 9. CÁC LỆNH LINUX HỆ THỐNG SỬ DỤNG (LINUX COMMAND SPECS)

Phân hệ gọi các công cụ quản trị hệ thống:
*   **`crontab`**:
    - `crontab -l`: Liệt kê cấu hình crontab của người dùng hiện thời.
    - `crontab -r`: Xóa bỏ tệp tin crontab của người dùng hiện thời.
    - `crontab -`: Nhận đầu vào từ Standard Input và ghi đè vào tệp crontab.
*   **`chmod`**: Cấp quyền thực thi (`+x`) cho tệp tin kịch bản.
*   **`cat`**: Ghép luồng dữ liệu cũ và mới.

---

## 10. THƯ VIỆN TIÊU CHUẨN SỬ DỤNG (STANDARD LIBRARIES)
Các tiêu đề thư viện C được sử dụng trong `shell_mgr.c`:

*   **`<unistd.h>`**: Cung cấp cuộc gọi hệ thống tạo tiến trình con `fork`, `execl`, và hàm kiểm tra quyền truy cập file `access`.
*   **`<sys/wait.h>`**: Cung cấp hàm `waitpid` và các macro giải mã trạng thái exit code.
*   **`<stdio.h>` & `<string.h>`**: Hỗ trợ xuất dữ liệu, định dạng chuỗi và so khớp chuỗi (`strcmp`).

---

## 11. AN NINH VÀ AN TOÀN LẬP LỊCH (SECURITY & SAFETY)

1.  **Chống chèn mã độc vào Crontab (Cron Injection Prevention):**
    - Khi ghép nối chuỗi cấu hình vào crontab, hệ thống kiểm tra sự tồn tại thực tế của tệp tin kịch bản bằng hàm `access(path, F_OK)` trong C hoặc lệnh `[ -f "$filePath" ]` trong Shell. Nếu tệp tin không tồn tại thực tế, hành động tạo tác vụ sẽ bị chặn lập tức, ngăn ngừa kẻ xấu chèn các lệnh độc hại (như chèn chuỗi `; rm -rf /`) vào cấu hình Crontab.
2.  **Ngăn ngừa lỗi phân quyền:**
    - Dự án cấu hình tác vụ Cron thông qua công cụ crontab mức người dùng (`user-level crontab`). Cơ chế này chạy kịch bản hoàn toàn dưới quyền của người dùng hiện tại (non-root), hạn chế tối đa các nguy cơ rò rỉ bảo mật làm ảnh hưởng đến tệp cấu hình hệ thống `/etc/crontab`.

---

## 12. HIỆU NĂNG VÀ TỐI ƯU HÓA (PERFORMANCE & OPTIMIZATION)

*   **Quản lý lỗi nhẹ nhàng (Quiet Error Handling):**
    - Sử dụng `crontab -l 2>/dev/null` giúp ngăn chặn việc trình vỏ in các thông báo lỗi rác (như `no crontab for user`) ra màn hình stdout khi crontab của người dùng trống, cải thiện thẩm mỹ giao diện TUI/CLI.
*   **Không tốn tài nguyên chạy ngầm:**
    - Ứng dụng C `sysmgr` chỉ đóng vai trò là cấu hình đầu vào. Toàn bộ chu trình đánh thức định kỳ và chạy ngầm kịch bản do daemon dịch vụ `cron` của Linux quản lý, không làm tiêu tốn bộ nhớ RAM hay chu kỳ CPU của `sysmgr`.

---

## 13. BẢN ĐỒ TRUY XUẤT YÊU CẦU BÀI TẬP (ASSIGNMENT TRACEABILITY)

| Mã yêu cầu bài tập | Nội dung yêu cầu | Trạng thái | Minh chứng trong kịch bản Shell & C |
| :--- | :--- | :--- | :--- |
| **REQ-CRON-01** | Liệt kê các tác vụ Cron đang được lập lịch trong hệ thống. | **Hoàn thành** | Lệnh `crontab -l` trong `shell/crontab.sh` và `shell_mgr.c:380`. |
| **REQ-CRON-02** | Tạo mới một tác vụ Cron với đường dẫn script và biểu thức thời gian tùy chỉnh. | **Hoàn thành** | Lệnh cập nhật crontab trong `crontab.sh:9` và `shell_mgr.c:441`. |
| **REQ-CRON-03** | Xóa sạch các cấu hình lập lịch Crontab của người dùng. | **Hoàn thành** | Lệnh `crontab -r` trong `crontab.sh:44` và `shell_mgr.c:382`. |

---

## 14. BẢN ĐỒ TRUY XUẤT TÀI LIỆU THAM KHẢO (REFERENCE TRACEABILITY)

*   **`Phan 2. T2.L2-P1_File.pdf` ➔ Trang 8 (Quản lý File Descriptor hệ thống):**
    - Ứng dụng chuyển hướng Standard Error (FD 2) sang thiết bị rỗng `/dev/null` để bỏ qua lỗi crontab rỗng. Triển khai trong [crontab.sh:9](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/crontab.sh#L9) và `shell_mgr.c:441`.

---

## 15. KIỂM THỬ VÀ CHẨN ĐOÁN LỖI (TEST SUITE & DIAGNOSTICS)
Phân hệ Task Scheduling được tích hợp kiểm thử trong [tests/shell_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/tests/shell_test.c).

*   **Quy trình kiểm thử sự tồn tại:**
    - Chạy lệnh `make test-shell` và thực thi `./tests/shell_test`. Bài test xác thực file kịch bản `shell/crontab.sh` tồn tại và có đầy đủ quyền đọc.
*   **Kiểm thử thực tế (Manual Integration Test):**
    1.  Tạo tác vụ Cron chạy kịch bản ghi nhịp tim mỗi phút:
        ```bash
        ./sysmgr cron create shell/test_cron.sh "* * * * *"
        ```
    2.  Xem danh sách tác vụ crontab hiện tại:
        ```bash
        ./sysmgr cron list
        ```
    3.  Chờ 1-2 phút và kiểm tra sự thay đổi trong file nhật ký nhịp tim:
        ```bash
        cat tmp/cron_test.log
        ```
        Nếu file xuất hiện các dòng `[CRON TEST] Executed successfully at ...` tức là phân hệ hoạt động hoàn toàn chính xác.
    4.  Xóa sạch các tác vụ để hoàn trả trạng thái ban đầu cho hệ thống:
        ```bash
        ./sysmgr cron delete
        ```
