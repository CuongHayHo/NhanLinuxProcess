#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Khởi tạo con trỏ đến cấu trúc ifaddrs
    struct ifaddrs *addresses;

    // Lấy danh sách các địa chỉ mạng
    if (getifaddrs(&addresses) == -1) {
        printf("Lỗi khi gọi hàm getifaddrs\n");
        return -1;
    }

    // Duyệt qua danh sách các địa chỉ mạng
    struct ifaddrs *address = addresses;
    while(address) {
        // Lấy Protocol Family (IPv4 hoặc IPv6)
        int family = address->ifa_addr->sa_family;
        if (family == AF_INET || family == AF_INET6) {
            // In ra tên giao diện mạng và loại giao thức
            printf("%s\t", address->ifa_name); //eth0, lo, ...
            printf("%s\t", family == AF_INET ? "IPv4" : "IPv6");

            // Lấy và in ra địa chỉ IP
            char ap[100];
            const int family_size = family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
            getnameinfo(address->ifa_addr, family_size, ap, sizeof(ap), 0, 0, NI_NUMERICHOST);
            // 0,0 - 2 tham số không được sử dụng, thường được sử dụng để lưu tên cổng và kích thước tên cổng
            //NI_NUMERICHOST chỉ định trả về địa chỉ mạng dưới dạng một chuỗi số
            printf("\t%s\n", ap);
        }

        // Chuyển sang địa chỉ tiếp theo
        address = address->ifa_next;
    }

    // Giải phóng bộ nhớ đã được cấp phát cho danh sách địa chỉ
    freeifaddrs(addresses);

    return 0;
}