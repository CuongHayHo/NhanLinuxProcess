/*
 * Minimal standalone example demonstrating getifaddrs()
 */
#include <stdio.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

int main(void) {
    struct ifaddrs *ifaddr, *ifa;

    printf("1. Calling getifaddrs()...\n");
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs failed");
        return 1;
    }

    printf("Active Interfaces:\n");
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }

        int family = ifa->ifa_addr->sa_family;
        printf("Interface: %-10s  Family: %s\n", ifa->ifa_name,
               (family == AF_INET) ? "IPv4" : 
               (family == AF_INET6) ? "IPv6" : 
               (family == AF_PACKET) ? "Link Layer" : "Other");
    }

    printf("2. Releasing ifaddrs list...\n");
    freeifaddrs(ifaddr);

    return 0;
}
