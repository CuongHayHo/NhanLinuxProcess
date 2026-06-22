# HƯỚNG DẪN KỸ THUẬT VÀ ĐẶC TẢ CHI TIẾT PHÂN HỆ SHELL PROGRAMMING & LAUNCHER (/shell)

Tài liệu này cung cấp đặc tả kỹ thuật, hướng dẫn lập trình, thiết kế kiến trúc và mô tả tích hợp chi tiết của phân hệ **Shell Programming & Launcher (`/shell`)** trong dự án **Linux System Manager (sysmgr)**. Đây là tài liệu tham chiếu kỹ thuật tối cao dành cho lập trình viên để duy trì, mở rộng và phát triển hệ thống lâu dài.

---

## BẢNG MỤC LỤC
1. [TỔNG QUAN PHÂN HỆ (MODULE OVERVIEW)](#1-tổng-quan-phân-hệ-module-overview)
2. [CÂY THƯ MỤC PHÂN HỆ (FILE TREE & INVENTORY)](#2-cây-thư-mục-phân-hệ-file-tree--inventory)
3. [MỐI LIÊN HỆ VỚI CÁC TÀI LIỆU LÝ THUYẾT NHÂN (REFERENCE PDFs)](#3-mối-liên-hệ-với-các-tài-liệu-lý-thuyết-nhân-reference-pdfs)
4. [TÍCH HỢP C VÀ SHELL SCRIPT (C & SHELL INTEGRATION)](#4-tích-hợp-c-và-shell-script-c--shell-integration)
5. [PHÂN TÍCH CHI TIẾT CÁC KỊCH BẢN SHELL (SHELL SCRIPT ANALYSIS)](#5-phân-tích-chi-tiết-các-kịch-bản-shell-shell-script-analysis)
6. [ĐẶC TẢ CHI TIẾT CÁC HÀM C WRPAPERS (C FUNCTION SPECIFICATIONS)](#6-đặc-tả-chi-tiết-các-hàm-c-wrappers-c-function-specifications)
7. [CÁC TÍNH NĂNG SHELL SCRIPTING (SHELL PROGRAMMING FEATURES)](#7-các-tính-năng-shell-scripting-shell-programming-features)
8. [CÁC LỆNH LINUX HỆ THỐNG ĐƯỢC SỬ DỤNG (LINUX COMMAND SPECS)](#8-các-lệnh-linux-hệ-thống-được-sử-dụng-linux-command-specs)
9. [THƯ VIỆN TIÊU CHUẨN TRONG MÃ C (STANDARD C LIBRARIES)](#9-thư-viện-tiêu-chuẩn-trong-mã-c-standard-c-libraries)
10. [KIẾN TRÚC VÀ LUỒNG ĐIỀU KHIỂN (SYSTEM ARCHITECTURE)](#10-kiến-trúc-vàng-luồng-điều-khiển-system-architecture)
11. [AN NINH VÀ AN TOÀN HỆ THỐNG (SECURITY & AUDIT)](#11-an-ninh-và-an-toàn-hệ-thống-security--audit)
12. [HIỆU NĂNG VÀ TỐI ƯU HÓA (PERFORMANCE & OPTIMIZATION)](#12-hiệu-năng-và-tối-ưu-hóa-performance--optimization)
13. [BẢN ĐỒ TRUY XUẤT YÊU CẦU BÀI TẬP (ASSIGNMENT TRACEABILITY)](#13-bản-đồ-truy-xuất-yêu-cầu-bài-tập-assignment-traceability)
14. [BẢN ĐỒ TRUY XUẤT TÀI LIỆU THAM KHẢO (REFERENCE TRACEABILITY)](#14-bản-đồ-truy-xuất-tài-liệu-tham-khảo-reference-traceability)
15. [KIỂM THỬ VÀ CHẨN ĐOÁN LỖI (TEST SUITE & DIAGNOSTICS)](#15-kiểm-thử-và-chẩn-đoán-lỗi-test-suite--diagnostics)

---

## 1. TỔNG QUAN PHÂN HỆ (MODULE OVERVIEW)
Phân hệ **Shell Programming & Launcher (`/shell`)** đóng vai trò là cầu nối cốt lõi và trung tâm tích hợp toàn bộ các kịch bản shell script tự động hóa với ứng dụng quản trị viết bằng ngôn ngữ C. 

Dự án áp dụng mô hình thiết kế lai **Hybrid Execution Architecture**: Sử dụng C để vẽ giao diện người dùng tương tác cao (TUI/CLI), xử lý logic cấu trúc, ghi nhận nhật ký (logging) và quản lý an ninh; đồng thời ủy thác (delegate) các thao tác hệ điều hành thô và quản trị thực tế (File Manager, Cron Job scheduling, NTP Time Sync, Package Manager) cho các kịch bản Bash shell script tương ứng ở không gian người dùng. Sự kết hợp này mang lại khả năng mở rộng nhanh chóng từ Shell scripting và tốc độ, sự an toàn, bảo mật từ ngôn ngữ C.

---

## 2. CÂY THƯ MỤC PHÂN HỆ (FILE TREE & INVENTORY)
Danh sách toàn bộ các tập tin cấu thành phân hệ Shell Programming bao gồm:

### A. Mã nguồn C (C Wrappers & Launcher)
1.  **[include/shell_mgr.h](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/include/shell_mgr.h)**:
    - *Vai trò:* Định nghĩa giao diện lập trình công khai (API) của Shell Manager để hệ thống Main hoặc CLI REPL gọi.
2.  **[modules/shell/shell_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/modules/shell/shell_mgr.c)**:
    - *Vai trò:* Triển khai logic gọi tiến trình thực thi lệnh/kịch bản qua cơ chế Fork-Exec, quản trị các biến môi trường và thiết lập luồng điều hướng CLI/TUI.

### B. Các kịch bản Shell backend hệ thống (`shell/`)
3.  **[shell/file.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/file.sh)**:
    - *Vai trò:* Nhận lệnh từ C và thực thi các thao tác File Manager (touch, cat, rm, mv, cp, stat, ls, mkdir, chmod, find, tar).
4.  **[shell/crontab.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/crontab.sh)**:
    - *Vai trò:* Kịch bản shell độc lập cho phép xem, tạo và xóa các tác vụ Cron hệ thống.
5.  **[shell/time.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/time.sh)**:
    - *Vai trò:* Quản lý menu thiết lập thời gian hệ thống và đồng bộ NTP trong môi trường shell độc lập.
6.  **[shell/autosync.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/autosync.sh)**:
    - *Vai trò:* Kịch bản đồng bộ thời gian tự phục hồi vượt qua rào cản DNSSEC khi clock hệ thống bị lệch sâu trong quá khứ.
7.  **[shell/program.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/program.sh)**:
    - *Vai trò:* Tự động nhận diện Trình quản lý gói của hệ thống (dnf/rpm hoặc apt/dpkg) và thực thi các tác vụ cài đặt, gỡ bỏ, truy vấn, chạy demo an toàn.
8.  **[shell/monitor.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/monitor.sh)**:
    - *Vai trò:* Kịch bản phụ trợ tương thích với Cron để ghi nhận thông tin phần cứng.
9.  **[shell/permission.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/permission.sh)**:
    - *Vai trò:* Kịch bản phụ trợ kiểm tra và phát hiện các tệp tin có quyền hạn không an toàn.
10. **[shell/update.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/update.sh)**:
    - *Vai trò:* Kịch bản dọn dẹp và cập nhật danh sách gói hệ thống.
11. **[shell/test_cron.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/test_cron.sh)**:
    - *Vai trò:* Kịch bản ghi timestamp để kiểm tra tính năng Cron hoạt động chính xác.

### C. Các kịch bản Shell giáo khoa (`scripts/`)
12. **[scripts/backup.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/scripts/backup.sh)**:
    - *Vai trò:* Kịch bản mô phỏng sao lưu thư mục cấu hình hệ thống `/etc`.
13. **[scripts/disk_usage.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/scripts/disk_usage.sh)**:
    - *Vai trò:* Kịch bản thống kê phân vùng và liệt kê tệp tin log dung lượng lớn nhất.
14. **[scripts/show_date.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/scripts/show_date.sh)**:
    - *Vai trò:* Kịch bản hiển thị thời gian, múi giờ và thời gian chạy máy (uptime).
15. **[scripts/hello_popup.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/scripts/hello_popup.sh)**:
    - *Vai trò:* Kịch bản bắn thông báo popup đồ họa GUI kiểm thử Cron task.

### D. Tập tin kiểm thử tự động
16. **[tests/shell_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/tests/shell_test.c)**:
    - *Vai trò:* Chương trình kiểm thử tự động toàn bộ API C và sự tồn tại của các kịch bản shell.

---

## 3. MỐI LIÊN HỆ VỚI CÁC TÀI LIỆU LÝ THUYẾT NHÂN (REFERENCE PDFs)
Phân hệ Shell Programming áp dụng trực tiếp kiến thức từ hai tài liệu lý thuyết cốt lõi:

### A. Tài liệu `Phan 2. T2.L2-P1_File.pdf` (Đọc ghi file trong Linux)
* **Khái niệm tương thích:** Quản lý luồng vào ra tiêu chuẩn, mô tả tệp tin (File descriptors) và các cơ chế chuyển hướng (Redirections) luồng dữ liệu.
* **Cách dự án áp dụng:**
  - Chuyển hướng đầu ra Standard Output và Standard Error sang thiết bị rỗng `/dev/null` hoặc ghi nối tiếp (append) vào file cấu hình:
    ```bash
    crontab -l 2>/dev/null | { cat; echo "$cronJobExpression $filePath"; } | crontab -
    ```
    Đây là ứng dụng trực tiếp của việc quản lý FD `2` (stderr) và FD `1` (stdout) được định nghĩa tại trang 8 của slide `File.pdf`.

### B. Tài liệu `Phan 2. T2.L2-P1_Process.pdf` (Quản lý Tiến trình)
* **Khái niệm tương thích:** Cơ chế nhân bản tiến trình con bằng `fork()`, nạp mới chương trình bằng `exec()` và đồng bộ hóa bằng `waitpid()`.
* **Cách dự án áp dụng:**
  - Hàm `shell_mgr_execute` áp dụng đúng quy trình chuẩn hóa: sinh tiến trình con, nạp trình vỏ lệnh shell `/bin/bash` hoặc `/bin/sh` truyền đối số rời rạc và cha đợi con hoàn tất để thu hồi mã lỗi (`exit code`).

---

## 4. TÍCH HỢP C VÀ SHELL SCRIPT (C & SHELL INTEGRATION)

Sự kết hợp giữa ngôn ngữ C và Shell script được thực thi thông qua mô hình tích hợp sau:

```
+-------------------------------------------------------------+
| C Application Context (TUI / REPL)                          |
| 1. Gọi shell_mgr_execute(cmd)                                |
| 2. open() & write() logs/system.log                          |
+-------------------------------------------------------------+
                              |
                              v
                      [ pipe / fork() ]
                              |
+-------------------------------------------------------------+
| Child Process Context                                       |
| 1. execl("/bin/bash", "bash", script_path, NULL)            |
|    hoặc execl("/bin/sh", "sh", "-c", command, NULL)          |
+-------------------------------------------------------------+
                              |
                              v
+-------------------------------------------------------------+
| Operating System Context                                    |
| 1. Thực thi kịch bản Bash Shell (ví dụ: program.sh)          |
| 2. Gọi các tiện ích hệ thống (apt, timedatectl, crontab...) |
| 3. Thoát tiến trình bằng exit(code)                         |
+-------------------------------------------------------------+
                              |
                              v
                       [ WEXITSTATUS ]
                              |
+-------------------------------------------------------------+
| C Parent Context                                            |
| 1. waitpid() nhận trạng thái kết thúc                       |
| 2. Nếu exit_code != 0, gọi log_error()                      |
+-------------------------------------------------------------+
```

### Các cặp hàm C gọi kịch bản Shell:
1.  **File Manager:** Hàm `file_mgr` trong C gọi `shell/file.sh` thông qua cơ chế fork-exec với các đối số tương ứng.
2.  **Cron scheduling:** Hàm C `shell_mgr_cron_execute` gọi các lệnh `crontab -l` và `crontab -r`.
3.  **System Time:** Hàm C `shell_mgr_time_execute` gọi kịch bản `shell/autosync.sh`.
4.  **Package Manager:** Hàm `package_mgr` trong C gọi kịch bản `shell/program.sh`.

---

## 5. PHÂN TÍCH CHI TIẾT CÁC KỊCH BẢN SHELL (SHELL SCRIPT ANALYSIS)

### 5.1. Kịch bản `shell/autosync.sh`
*   **Mục đích:** Tự động đồng bộ hóa thời gian hệ thống một cách an toàn. Giải quyết bài toán DNSSEC validation failed khi đồng hồ hệ thống bị lệch lùi sâu về quá khứ khiến không thể phân giải DNS các máy chủ NTP.
*   **Tham số đầu vào:** Không có.
*   **Các biến quan trọng:**
    - `IPs`: Mảng chứa các địa chỉ IP của DNS public có độ tin cậy cao (`1.1.1.1`, `8.8.8.8`, `9.9.9.9`, `208.67.222.222`).
    - `HTTP_DATE`: Chuỗi lưu thời gian định dạng GMT lấy được từ header HTTP Date.
*   **Luồng xử lý chi tiết:**
    - Duyệt qua mảng `IPs`. Với mỗi IP, gọi lệnh `curl` gửi yêu cầu HTTP HEAD tới cổng 80:
      ```bash
      header_date=$(curl -I -s --connect-timeout 3 "http://$ip" | grep -i "^date:" | cut -d' ' -f2- | tr -d '\r')
      ```
      Việc này bypass hoàn toàn hệ thống phân giải tên miền DNS và xác thực SSL/TLS.
    - Dùng lệnh `date -d "$header_date"` kiểm tra định dạng thời gian trả về. Nếu hợp lệ, gán vào `HTTP_DATE` và thoát vòng lặp.
    - Cập nhật thời gian hệ thống gần đúng bằng lệnh: `sudo date -s "$HTTP_DATE"`.
    - Sau khi thời gian hệ thống ở mức gần đúng, DNSSEC sẽ tự động pass. Script tiến hành bước đồng bộ chính xác cao bằng cách tìm và gọi các tiện ích NTP trên hệ thống theo thứ tự ưu tiên:
      1. `chronyc makestep` (nếu có `chrony`).
      2. `ntpdate pool.ntp.org` (nếu có `ntpdate`).
      3. `timedatectl set-ntp true` (nếu có `timedatectl`).
    - **Mã thoát (Exit codes):**
      - `0`: Đồng bộ NTP thành công hoặc kiểm tra năm hiện tại `>= 2026` thành công.
      - `1`: Không kết nối được internet hoặc không thể ghi đè thời gian (thiếu quyền sudo).
      - `2`: NTP lỗi nhưng đã cập nhật được theo HTTP Date.

### 5.2. Kịch bản `shell/program.sh`
*   **Mục đích:** Tự động cài đặt và gỡ bỏ các chương trình phần mềm, phục vụ như là backend cho phân hệ Package Manager.
*   **Tham số đầu vào:**
    - `$1` (`ACTION`): Hành động yêu cầu (`search`, `info`, `install`, `remove`, `demo`, `setup`).
    - `$2` (`PKG`): Tên gói phần mềm mục tiêu.
*   **Luồng xử lý chi tiết:**
    - **Nhận diện trình quản lý gói:** Kiểm tra sự tồn tại của `/usr/bin/dnf` và `/usr/bin/apt-get` để thiết lập biến cờ `PM` (là `dnf` hoặc `apt`) và biến truy vấn `QUERY` (là `rpm` hoặc `dpkg`).
    - **Bảo mật cài đặt:** Chặn cài đặt hoặc gỡ bỏ các gói nhân hệ thống quan trọng để tránh gây lỗi OS (như `kernel`, `glibc`, `bash`, `systemd`, `gcc`, `dnf`, `rpm`, `python`) bằng lệnh `case` khớp mẫu.
    - **Hành động `search`:** Gọi `rpm -qa` hoặc `dpkg-query -W` kết hợp với `grep -i`.
    - **Hành động `info`:** Truy xuất thông tin từ gói đã cài hoặc gọi `dnf info`/`apt-cache show` cho gói chưa cài.
    - **Hành động `install`/`remove`:** Gọi `sudo dnf install/remove -y` hoặc `sudo apt-get install/remove -y`.
    - **Hành động `demo`:** Tự động chọn một gói demo vô hại từ danh sách (`hello`, `sl`, `figlet`, `tree`, `jq`), kiểm tra gói nào chưa được cài, thực hiện tuần tự: Search -> Info -> Install -> Verify (Kiểm tra cài thành công) -> Remove -> Verify Removal (Kiểm tra hệ thống đã sạch). Ghi nhật ký từng bước vào `logs/system.log`.

### 5.3. Kịch bản `shell/crontab.sh`
*   **Mục đích:** Cung cấp trình đơn tương tác cấu hình các scheduled tasks mức người dùng.
*   **Luồng xử lý chi tiết:**
    - **Hành động Tạo (`createCronJob`):** Nhận đường dẫn tệp script và chuỗi biểu thức Cron. Cấp quyền thực thi bằng `chmod +x` trên script, sau đó ghi nối tiếp vào crontab:
      ```bash
      crontab -l | { cat; echo "$cronJobExpression $filePath"; } | crontab -
      ```
    - **Hành động Xóa (`crontab -r`):** Gỡ bỏ toàn bộ danh sách cron hiện hành của người dùng.
    - **Hành động Liệt kê (`listCronJobs`):** In ra nội dung crontab hiện thời thông qua lệnh `crontab -l`.

### 5.4. Kịch bản `shell/time.sh`
*   **Mục đích:** Trình đơn điều phối thiết lập thời gian hệ thống thủ công và đồng bộ Internet.
*   **Đầu vào:** Lựa chọn tương tác từ bàn phím.
*   **Luồng xử lý:** Kiểm tra định dạng chuỗi nhập vào bằng biểu thức chính quy regex: `^[0-9]{4}-[0-9]{2}-[0-9]{2}\ [0-9]{2}:[0-9]{2}:[0-9]{2}$`. Nếu hợp lệ, cho phép ghi đè thời gian qua lệnh `sudo date -s` (ở lựa chọn Apply) hoặc hiển thị mô phỏng (ở lựa chọn Learning Mode).

---

## 6. ĐẶC TẢ CHI TIẾT CÁC HÀM C WRAPPERS (C FUNCTION SPECIFICATIONS)

### 6.1. Hàm `shell_mgr_execute`
*   **Nguyên mẫu (Prototype):** `int shell_mgr_execute(const char* command);`
*   **Tệp nguồn / Tiêu đề:** [shell_mgr.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/modules/shell/shell_mgr.c#L596) / [shell_mgr.h](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/include/shell_mgr.h#L21)
*   **Mục đích:** Thực thi an toàn một câu lệnh thô hoặc tệp kịch bản bash từ môi trường C thông qua cơ chế tạo tiến trình con.
*   **Chi tiết mã nguồn:**
  ```c
  int shell_mgr_execute(const char* command) {
      pid_t pid;
      int is_script = 0;
      const char* script_path = NULL;

      if (!command || strlen(command) == 0) {
          return -1;
      }

      if (strncmp(command, "/bin/bash ", 10) == 0) {
          is_script = 1;
          script_path = command + 10;
          log_info("SHELL", "Script selected: '%s'", script_path);
      } else {
          log_info("SHELL", "Command entered: '%s'", command);
      }

      pid = fork();
      if (pid < 0) {
          log_error("SHELL", "Errors: fork failed (errno %d)", errno);
          return -1;
      }

      if (pid == 0) {
          /* Tiến trình con thực thi */
          if (is_script) {
              execl("/bin/bash", "bash", script_path, (char*)NULL);
              fprintf(stderr, "Error: Script '%s' not found...\n", script_path);
          } else {
              execl("/bin/sh", "sh", "-c", command, (char*)NULL);
              fprintf(stderr, "Error: Command execution failed...\n");
          }
          exit(127);
      } else {
          /* Tiến trình cha chờ đợi */
          int status;
          log_info("SHELL", "Child process created (PID: %d)", pid);

          if (waitpid(pid, &status, 0) == -1) {
              log_error("SHELL", "Errors: waitpid failed (errno %d)", errno);
              return -1;
          }

          if (WIFEXITED(status)) {
              int exit_code = WEXITSTATUS(status);
              log_info("SHELL", "Exit status: %d", exit_code);
              if (exit_code != 0) {
                  log_error("SHELL", "Errors: execution failed with exit code %d", exit_code);
              }
              return exit_code;
          } else if (WIFSIGNALED(status)) {
              int sig = WTERMSIG(status);
              log_error("SHELL", "Errors: command terminated by signal %d", sig);
              return -1;
          }
      }
      return -1;
  }
  ```
  - **Diễn giải thuật toán:**
    - Hàm phân tích chuỗi `command`. Nếu chuỗi bắt đầu bằng `/bin/bash `, hàm đánh dấu `is_script = 1` và tách đường dẫn kịch bản. Ghi log sự kiện thông qua `log_info`.
    - Gọi cuộc gọi hệ thống `fork()`.
    - Tiến trình con (`pid == 0`) nạp chương trình bằng `execl()`. Nếu chạy script, nạp trực tiếp `/bin/bash` và truyền tham số `script_path`. Cách thiết kế này an toàn hơn so với việc thông dịch qua shell `-c` của `/bin/sh` vì kịch bản được thực thi trực tiếp làm đối số đầu vào của trình vỏ. Nếu nạp thất bại, con thoát bằng mã lỗi `127`.
    - Tiến trình cha (`pid > 0`) gọi `waitpid` chặn luồng để chờ tiến trình con hoàn tất nhiệm vụ. Giải mã trạng thái thoát của con bằng macro `WEXITSTATUS(status)`. Nếu exit_code khác `0`, ghi log lỗi dạng ERROR vào hệ thống nhật ký log.

### 6.2. Hàm `shell_mgr_env_run`
*   **Nguyên mẫu:** `void shell_mgr_env_run(void);`
*   **Tệp nguồn / Tiêu đề:** `shell_mgr.c` / `shell_mgr.h`
*   **Mục đích:** Cung cấp menu TUI/CLI cho phép liệt kê, tìm kiếm, thiết lập tạm thời và gỡ bỏ các biến môi trường của tiến trình hiện tại.
*   **Chi tiết luồng xử lý:**
    - **Lựa chọn 1 (List):** Duyệt mảng cứng các biến môi trường quan trọng (`PATH`, `HOME`, `USER`, `PWD`, `SHELL`, `LANG`, `TERM`) và in giá trị tương ứng trả về từ thư viện `getenv()`.
    - **Lựa chọn 2 (Query):** Nhận tên biến từ người dùng, gọi `getenv()`. Nếu biến tồn tại, in giá trị và ghi log INFO; ngược lại, thông báo biến chưa được set.
    - **Lựa chọn 3 (Set):** Nhập tên và giá trị biến, gọi hàm thư viện chuẩn **`setenv(name, value, 1)`** (tham số thứ ba bằng `1` để ghi đè nếu biến đã tồn tại). Nếu thành công, ghi log INFO; nếu thất bại, ghi log ERROR.
    - **Lựa chọn 4 (Remove):** Gọi **`unsetenv(name)`** để xóa biến ra khỏi danh sách môi trường của tiến trình.

### 6.3. Hàm `shell_mgr_cron_execute`
*   **Nguyên mẫu:** `void shell_mgr_cron_execute(int argc, char** argv);`
*   **Tệp nguồn / Tiêu đề:** `shell_mgr.c` / `shell_mgr.h`
*   **Mục đích:** Xử lý các lệnh cấu hình tác vụ Cron từ REPL hoặc menu TUI.
*   **Chi tiết luồng xử lý:**
    - Nếu có tham số dòng lệnh (`argc > 1`):
      - Nhận diện hành động: `list` (gọi `shell_mgr_execute("crontab -l")`), `delete` (gọi `shell_mgr_execute("crontab -r")`), `create` (nhận tham số path và expression từ `argv` hoặc linenoise, chạy lệnh `chmod +x` trên script mục tiêu, sau đó cập nhật Crontab thông qua pipeline chuyển hướng an toàn).
    - Nếu chạy tương tác (TUI): Vẽ menu các lựa chọn Xem, Tạo, Xóa và thực thi các cuộc gọi shell tương tự.

### 6.4. Hàm `shell_mgr_time_execute`
*   **Nguyên mẫu:** `void shell_mgr_time_execute(int argc, char** argv);`
*   **Tệp nguồn / Tiêu đề:** `shell_mgr.c` / `shell_mgr.h`
*   **Mục đích:** Xử lý cấu hình thời gian hệ thống và kích hoạt đồng bộ hóa tự phục hồi.
*   **Chi tiết luồng xử lý:**
    - Phân tích cú pháp đối số dòng lệnh:
      - `show`: Gọi lệnh hệ thống `date`.
      - `zone`: Kiểm tra xem `/usr/bin/timedatectl` có tồn tại hay không. Nếu có, thực thi `timedatectl | grep 'Time zone'`; ngược lại, đọc file `/etc/timezone`.
      - `set`: Kiểm tra và định dạng chuỗi datetime, thực thi câu lệnh phân quyền nâng cao `sudo date -s "YYYY-MM-DD HH:MM:SS"`.
      - `sync`: Thực thi kịch bản đồng bộ tự phục hồi `shell/autosync.sh` thông qua hàm `shell_mgr_execute`.

---

## 7. CÁC TÍNH NĂNG SHELL SCRIPTING (SHELL PROGRAMMING FEATURES)
Các cấu trúc lập trình shell được áp dụng nhuần nhuyễn trong hệ thống kịch bản:

1.  **Case statements:** Sử dụng để định tuyến hành động (routing actions) trong `file.sh`, `program.sh`, `time.sh`, và `crontab.sh` giúp mã nguồn rõ ràng hơn nhiều so với chuỗi `if-else` lồng nhau.
2.  **Mảng dữ liệu (Arrays):** Sử dụng lưu trữ danh sách máy chủ IP dự phòng trong `autosync.sh` (`IPs`) và danh sách gói demo trong `program.sh` (`candidates`, `fallbacks`), duyệt mảng bằng cú pháp `"${IPs[@]}"`.
3.  **Command substitution:** Trích xuất kết quả lệnh gán vào biến:
    ```bash
    USER_ID=$(id -u)
    ```
4.  **Chuyển hướng & Đường ống (Pipelines & Redirections):** Ghép nối dữ liệu để thêm tác vụ vào crontab mà không ghi đè các tác vụ cũ:
    ```bash
    crontab -l 2>/dev/null | { cat; echo "$cronJobExpression $filePath"; } | crontab -
    ```
5.  **Quoting (Bao ngoặc kép):** Luôn bao bọc các biến đường dẫn bằng ngoặc kép (ví dụ: `"$ARG1"`, `"$filePath"`) để tránh lỗi biên dịch của trình vỏ lệnh shell khi đường dẫn chứa các ký tự đặc biệt hoặc dấu khoảng trắng.

---

## 8. CÁC LỆNH LINUX HỆ THỐNG ĐƯỢC SỬ DỤNG (LINUX COMMAND SPECS)

Phân hệ gọi trực tiếp nhiều công cụ và tiện ích dòng lệnh Linux:

| Lệnh Linux | Cú pháp sử dụng trong dự án | Mục đích sử dụng | Vị trí kịch bản |
| :--- | :--- | :--- | :--- |
| **`date`** | `date -s "YYYY-MM-DD HH:MM:SS"` | Hiển thị hoặc thiết lập thời gian hệ thống. | [time.sh:62](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/time.sh#L62) |
| **`timedatectl`** | `timedatectl | grep "Time zone"` | Kiểm tra trạng thái múi giờ và cấu hình NTP. | [time.sh:18](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/time.sh#L18) |
| **`chronyc`** | `chronyc makestep` | Thực hiện đồng bộ hóa bước thời gian ngay lập tức. | [autosync.sh:56](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/autosync.sh#L56) |
| **`ntpdate`** | `ntpdate pool.ntp.org` | Đồng bộ thời gian với máy chủ NTP. | [autosync.sh:69](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/autosync.sh#L69) |
| **`crontab`** | `crontab -l` / `crontab -r` | Liệt kê hoặc xóa các cấu hình lập lịch Cron. | [crontab.sh:24](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/crontab.sh#L24) |
| **`apt-get`** | `apt-get install/remove -y` | Quản lý gói cài đặt trên Debian/Ubuntu. | [program.sh:93](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/program.sh#L93) |
| **`dnf`** | `dnf install/remove -y` | Quản lý gói cài đặt trên RHEL/CentOS/Fedora. | [program.sh:91](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/program.sh#L91) |
| **`dpkg-query`** | `dpkg-query -W` / `dpkg-query -s` | Truy vấn cơ sở dữ liệu các gói cài Debian. | [program.sh:50](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/program.sh#L50) |
| **`rpm`** | `rpm -qa` / `rpm -q` | Truy vấn cơ sở dữ liệu các gói cài RedHat. | [program.sh:48](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/program.sh#L48) |
| **`curl`** | `curl -I -s --connect-timeout 3` | Gửi HTTP HEAD request để lấy thời gian máy chủ. | [autosync.sh:23](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/autosync.sh#L23) |
| **`chmod`** | `chmod +x filePath` | Cấp quyền thực thi cho tập tin script. | [crontab.sh:8](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/crontab.sh#L8) |

---

## 9. THƯ VIỆN TIÊU CHUẨN TRONG MÃ C (STANDARD C LIBRARIES)
Các tiêu đề thư viện C được sử dụng trong `shell_mgr.c`:

*   **`<unistd.h>`**: Cung cấp cuộc gọi hệ thống quản lý tiến trình `fork`, `execl`, và hàm kiểm tra quyền truy cập file `access`.
*   **`<sys/wait.h>`**: Cung cấp hàm `waitpid` và các macro giải mã trạng thái exit code của kịch bản shell con.
*   **`<stdlib.h>`**: Cung cấp hàm làm việc với môi trường tiến trình `getenv`, `setenv`, `unsetenv` và hàm chuyển đổi chuỗi `strtol`.
*   **`<stdio.h>` & `<string.h>`**: Hỗ trợ xuất dữ liệu, định dạng chuỗi (`snprintf`) và sao chép an toàn (`strncpy`).

---

## 10. KIẾN TRÚC VÀ LUỒNG ĐIỀU KHIỂN (SYSTEM ARCHITECTURE)

### A. Quy trình chạy lệnh CLI thô (Raw Command Workflow)
Khi người dùng nhập câu lệnh thô (ví dụ: `ls -l`):
1.  Hàm `shell_mgr_execute` được kích hoạt.
2.  Tiến trình cha `fork()` ra con.
3.  Tiến trình con gọi `execl("/bin/sh", "sh", "-c", "ls -l", (char*)NULL)`.
4.  Nhân Linux nạp trình vỏ `/bin/sh`, thông dịch và chạy lệnh `ls -l`.
5.  Cha đợi con bằng `waitpid()`, nhận trạng thái kết thúc và ghi nhật ký log.

### B. Quy trình chạy Kịch bản Shell (Script Execution Workflow)
Khi người dùng chọn chạy kịch bản (ví dụ: `shell/autosync.sh`):
1.  Hàm `shell_mgr_execute` nhận diện tiền tố `/bin/bash `.
2.  Tách đường dẫn kịch bản thành `shell/autosync.sh`.
3.  Tiến trình con gọi: `execl("/bin/bash", "bash", "shell/autosync.sh", (char*)NULL)`.
4.  Nhân nạp trực tiếp `/bin/bash` chạy tệp kịch bản. Việc nạp trực tiếp tệp làm đối số giúp tránh được các lỗi bảo mật chèn câu lệnh.

---

## 11. AN NINH VÀ AN TOÀN HỆ THỐNG (SECURITY & AUDIT)

1.  **Chặn cài đặt/gỡ bỏ Gói nhân (Critical Package Blocking):**
    - Kịch bản `program.sh` bảo vệ hệ điều hành khỏi các thao tác phá hủy vô ý bằng cách chặn toàn bộ hành động cài đặt/gỡ bỏ đối với các gói khớp mẫu: `kernel*`, `glibc*`, `bash*`, `systemd*`, `gcc*`, `dnf*`, `rpm*`, `python*`. Khi phát hiện, script in lỗi trực tiếp ra stdout và thoát với mã lỗi `1`, đồng thời ghi nhận sự kiện cảnh báo bảo mật vào `system.log`.
2.  **Tránh lỗi Shell Injection khi gọi kịch bản:**
    - Bằng cách sử dụng **`execl`** truyền các tham số tách biệt rõ ràng thay vì hệ thống chuỗi nối tiếp thô, ứng dụng C đảm bảo các đối số kịch bản truyền vào không bao giờ được thông dịch ngược lại thành câu lệnh chạy mới.
3.  **Hạn chế quyền hạn (Least Privilege):**
    - Các thao tác yêu cầu đặc quyền quản trị (cấu hình giờ hệ thống, cài đặt phần mềm) sử dụng lệnh `sudo`. Nếu người chạy không nằm trong tệp cấu hình `/etc/sudoers`, câu lệnh sẽ tự động thoát với lỗi trả về từ hệ điều hành, đảm bảo hệ thống không bị leo thang quyền hạn trái phép.

---

## 12. HIỆU NĂNG VÀ TỐI ƯU HÓA (PERFORMANCE & OPTIMIZATION)

*   **Tối ưu hóa nạp kịch bản (Direct Shell Loading):**
    - Chạy trực tiếp `/bin/bash` và truyền tệp kịch bản làm đối số đầu vào thay vì nạp trung gian qua `/bin/sh -c` giúp hệ thống bớt đi một cấp độ tạo tiến trình con phụ, tăng tốc độ khởi chạy kịch bản và giảm mức độ tiêu hao CPU của hệ thống.
*   **Bypass DNSSEC (DNS SECurity) trong autosync.sh:**
    - Thay vì gửi truy vấn tên miền DNS (thường bị treo lâu khi đồng hồ hệ thống sai lệch sâu khiến chứng thư SSL hết hạn), script gửi kết nối trực tiếp đến IP thô. Việc này giúp giảm thời gian chờ đợi lỗi phân giải (DNS timeout) từ vài phút xuống còn dưới 3 giây.

---

## 13. BẢN ĐỒ TRUY XUẤT YÊU CẦU BÀI TẬP (ASSIGNMENT TRACEABILITY)

| Mã yêu cầu bài tập | Nội dung yêu cầu | Trạng thái | Minh chứng trong kịch bản Shell & C |
| :--- | :--- | :--- | :--- |
| **REQ-SH-01** | Lập trình kịch bản shell để quản lý tệp tin và thư mục (File Management Launcher). | **Hoàn thành** | Kịch bản [shell/file.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/file.sh) xử lý 13 thao tác và được gọi qua launcher C. |
| **REQ-SH-02** | Lập trình kịch bản shell cài đặt/gỡ bỏ chương trình tự động (Package Management). | **Hoàn thành** | Kịch bản [shell/program.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/program.sh) tự động nhận diện PM và cài đặt phần mềm. |
| **REQ-SH-03** | Thiết lập tác vụ lập lịch định kỳ tự động (Cron job management). | **Hoàn thành** | Kịch bản [shell/crontab.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/crontab.sh) và hàm C `shell_mgr_cron_execute`. |
| **REQ-SH-04** | Thay đổi múi giờ và cấu hình đồng bộ hóa thời gian hệ thống tự phục hồi. | **Hoàn thành** | Kịch bản [shell/autosync.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/autosync.sh) và [shell/time.sh](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/time.sh). |
| **REQ-SH-05** | Quản lý các biến môi trường của tiến trình hiện tại. | **Hoàn thành** | Hàm C `shell_mgr_env_run` gọi trực tiếp `setenv`, `unsetenv`, `getenv`. |

---

## 14. BẢN ĐỒ TRUY XUẤT TÀI LIỆU THAM KHẢO (REFERENCE TRACEABILITY)

*   **`Phan 2. T2.L2-P1_File.pdf` ➔ Trang 8 (Bảng mô tả tệp của tiến trình):**
    - Dự án áp dụng chuyển hướng Standard Error và Standard Output của crontab thô vào đường dẫn để cập nhật Crontab an toàn:
      ```bash
      crontab -l 2>/dev/null | { cat; echo "$expr $path"; } | crontab -
      ```
      Thao tác này được viết trong [crontab.sh:9](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/shell/crontab.sh#L9) và `shell_mgr.c:441`.
*   **`Phan 2. T2.L2-P1_Process.pdf` ➔ Chương 2 (Vòng đời tiến trình Fork-Exec):**
    - Triển khai thuật toán Fork-Exec nạp kịch bản shell bằng `execl` và giải mã exit status trong [shell_mgr.c:614-656](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/modules/shell/shell_mgr.c#L614).

---

## 15. KIỂM THỬ VÀ CHẨN ĐOÁN LỖI (TEST SUITE & DIAGNOSTICS)
Phân hệ tích hợp chương trình kiểm thử tự động toàn diện tại [tests/shell_test.c](file:///home/cuonghayho/Documents/ThamKhaoPRJLapTrinhNhan/PRJ/tests/shell_test.c).

### A. Hướng dẫn chạy Test:
Biên dịch từ thư mục gốc của dự án:
```bash
make test-shell
```
Chạy kiểm thử:
```bash
./tests/shell_test
```

### B. Các kịch bản kiểm thử được thực thi:
1.  **Raw Command Execution Tests (Section 1):**
    - Chạy thử lệnh `pwd` và xác nhận mã thoát trả về là `0`.
    - Chạy thử lệnh `whoami` và xác nhận thành công.
    - Chạy lệnh `ls` liệt kê thư mục.
    - Chạy thử một lệnh lỗi `invalid_command_xyz123`, xác nhận mã thoát trả về đúng bằng **`127`** (Command not found).
2.  **Script Runner Tests (Section 2):**
    - Gọi thử kịch bản `/bin/bash scripts/backup.sh` xác nhận mã thoát là `0`.
    - Gọi thử kịch bản `/bin/bash scripts/disk_usage.sh` và `show_date.sh`.
    - Gọi một kịch bản không tồn tại `invalid_script.sh`, xác nhận mã thoát trả về là `127`.
3.  **Environment Manager Tests (Section 3):**
    - Kiểm tra đọc biến môi trường mặc định hệ thống `PATH`.
    - Gọi `setenv("TEST_ENV_VAR", "sprint3_value", 1)` và xác nhận ghi nhận thành công giá trị.
    - Gọi `setenv` ghi đè có kiểm soát (`overwrite = 0` và `overwrite = 1`).
    - Gọi `unsetenv("TEST_ENV_VAR")` và xác nhận biến đã được giải phóng sạch sẽ (trả về `NULL`).
4.  **Script Presence Sanity Checks (Section 4):**
    - Kiểm tra sự tồn tại và quyền đọc (`access` với `F_OK` và `R_OK`) trên toàn bộ 4 kịch bản hệ thống cốt lõi: `shell/file.sh`, `shell/crontab.sh`, `shell/time.sh`, và `shell/program.sh`.
5.  **Tích hợp Logger:** Kiểm thử việc ghi nhận lịch sử lệnh và ghi lỗi khi chạy sai câu lệnh vào tệp tin nhật ký `logs/system.log`.
