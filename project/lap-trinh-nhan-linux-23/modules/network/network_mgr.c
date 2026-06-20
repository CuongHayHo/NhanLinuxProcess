/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/network/network_mgr.c
 * Purpose: Linux Network Interface Manager and diagnostics.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <net/if_arp.h>
#include "network_mgr.h"
#include "logger.h"
#include "ui.h"

/* Static helper to convert hex string IP to standard dotted-decimal string */
static void hex_to_ip(const char* hex, char* ip_out) {
    unsigned long val;
    if (sscanf(hex, "%lx", &val) == 1) {
        struct in_addr addr;
        addr.s_addr = (uint32_t)val;
        if (inet_ntop(AF_INET, &addr, ip_out, INET_ADDRSTRLEN) == NULL) {
            strcpy(ip_out, "0.0.0.0");
        }
    } else {
        strcpy(ip_out, "0.0.0.0");
    }
}

/* Static process execution helper */
static int run_command_direct(const char *wdir, const char *cmd, char *const argv[], int *exit_code) {
    pid_t pid = fork();
    if (pid == -1) {
        log_error("NETWORK", "fork failed: %s", strerror(errno));
        return -1;
    }
    if (pid == 0) {
        if (wdir) {
            if (chdir(wdir) == -1) {
                exit(errno);
            }
        }
        execvp(cmd, argv);
        exit(errno);
    }
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        log_error("NETWORK", "waitpid failed: %s", strerror(errno));
        return -1;
    }
    if (WIFEXITED(status)) {
        if (exit_code) {
            *exit_code = WEXITSTATUS(status);
        }
        return 0;
    }
    return -1;
}

/* Helper to read stat values from sysfs statistics */
static long long read_sysfs_stat(const char* iface, const char* stat_name) {
    char path[256];
    snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/%s", iface, stat_name);
    FILE* fp = fopen(path, "r");
    if (!fp) return -1;
    long long val = 0;
    if (fscanf(fp, "%lld", &val) != 1) {
        val = -1;
    }
    fclose(fp);
    return val;
}

/* Helper to prompt for ENTER */
static void network_menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Menu input reader */
static int read_network_choice(void) {
    char input_buf[128];
    char* endptr;
    long val;

    if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
        return -1;
    }

    input_buf[strcspn(input_buf, "\n")] = '\0';
    if (strlen(input_buf) == 0) {
        return -1;
    }

    val = strtol(input_buf, &endptr, 10);
    if (*endptr != '\0') {
        return -1;
    }
    return (int)val;
}

/* 1. List Network Interfaces (Clean name list for option 1) */
int network_list_interfaces_clean(void) {
    struct ifaddrs *ifaddr, *ifa;
    char processed[64][IFNAMSIZ];
    int processed_count = 0;

    log_info("NETWORK", "Listing interface names only");

    if (getifaddrs(&ifaddr) == -1) {
        log_error("NETWORK", "API failure: getifaddrs() failed (errno %d)", errno);
        printf("Error: Failed to retrieve network interfaces.\n");
        return -1;
    }

    printf("\n--- Available Network Interfaces ---\n");
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_name == NULL) {
            continue;
        }

        /* Check for duplicates */
        int already_processed = 0;
        for (int i = 0; i < processed_count; ++i) {
            if (strcmp(processed[i], ifa->ifa_name) == 0) {
                already_processed = 1;
                break;
            }
        }
        if (already_processed) {
            continue;
        }

        /* Register as processed */
        if (processed_count < 64) {
            strncpy(processed[processed_count], ifa->ifa_name, IFNAMSIZ - 1);
            processed[processed_count][IFNAMSIZ - 1] = '\0';
            processed_count++;
        }

        printf("%s\n", ifa->ifa_name);
    }
    printf("------------------------------------\n");

    freeifaddrs(ifaddr);
    return 0;
}

/* 2. Detailed Interface Information */
int network_interface_info(void) {
    char iface[64];
    struct ifaddrs *ifaddr, *ifa;
    int found = 0;
    unsigned int flags = 0;

    /* Display clean list first for convenience */
    network_list_interfaces_clean();

    printf("Enter interface name: ");
    fflush(stdout);
    if (fgets(iface, sizeof(iface), stdin) == NULL) return -1;
    iface[strcspn(iface, "\n")] = '\0';
    if (strlen(iface) == 0) return -1;

    log_info("NETWORK", "Interface details queried for %s", iface);

    if (getifaddrs(&ifaddr) == -1) {
        log_error("NETWORK", "API failure: getifaddrs() failed (errno %d)", errno);
        printf("Error: Failed to retrieve network interfaces.\n");
        return -1;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_name && strcmp(ifa->ifa_name, iface) == 0) {
            found = 1;
            flags = ifa->ifa_flags;
            break;
        }
    }

    if (!found) {
        printf("Error: Interface '%s' not found.\n", iface);
        freeifaddrs(ifaddr);
        return -1;
    }

    printf("\n========================================\n");
    printf("Interface Name: %s\n", iface);
    printf("State:          %s\n", (flags & IFF_UP) ? "UP" : "DOWN");

    /* Print IPv4 and IPv6 addresses */
    int ipv4_count = 0;
    int ipv6_count = 0;
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_name && strcmp(ifa->ifa_name, iface) == 0 && ifa->ifa_addr) {
            int family = ifa->ifa_addr->sa_family;
            if (family == AF_INET) {
                char ip_str[INET_ADDRSTRLEN];
                struct sockaddr_in* addr = (struct sockaddr_in*)ifa->ifa_addr;
                if (inet_ntop(AF_INET, &addr->sin_addr, ip_str, sizeof(ip_str)) != NULL) {
                    printf("IPv4:           %s\n", ip_str);
                    ipv4_count++;
                }
            } else if (family == AF_INET6) {
                char ip_str[INET6_ADDRSTRLEN];
                struct sockaddr_in6* addr = (struct sockaddr_in6*)ifa->ifa_addr;
                if (inet_ntop(AF_INET6, &addr->sin6_addr, ip_str, sizeof(ip_str)) != NULL) {
                    printf("IPv6:           %s\n", ip_str);
                    ipv6_count++;
                }
            }
        }
    }
    if (ipv4_count == 0) printf("IPv4:           N/A\n");
    if (ipv6_count == 0) printf("IPv6:           N/A\n");

    freeifaddrs(ifaddr);

    /* MTU and MAC hardware address using standard C sockets/ioctl */
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int mtu = -1;
    char mac_str[32] = "N/A";
    if (sock >= 0) {
        struct ifreq ifr;
        strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';

        if (ioctl(sock, SIOCGIFMTU, &ifr) == 0) {
            mtu = ifr.ifr_mtu;
        }

        if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
            unsigned char* mac = (unsigned char*)ifr.ifr_hwaddr.sa_data;
            snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        }
        close(sock);
    }

    printf("MAC Address:    %s\n", mac_str);
    if (mtu >= 0) {
        printf("MTU:            %d\n", mtu);
    } else {
        printf("MTU:            N/A\n");
    }

    /* Packet transmission stats from sysfs */
    long long rx_packets = read_sysfs_stat(iface, "rx_packets");
    long long tx_packets = read_sysfs_stat(iface, "tx_packets");

    if (rx_packets >= 0) {
        printf("RX packets:     %lld\n", rx_packets);
    } else {
        printf("RX packets:     N/A\n");
    }

    if (tx_packets >= 0) {
        printf("TX packets:     %lld\n", tx_packets);
    } else {
        printf("TX packets:     N/A\n");
    }
    printf("========================================\n");

    return 0;
}

/* 3. Configure Interface (Learning Mode Only) */
void network_configure_interface_learning(void) {
    char iface[64];
    char ip_cidr[64];

    printf("Enter interface name (e.g. eth0): ");
    fflush(stdout);
    if (fgets(iface, sizeof(iface), stdin) == NULL) return;
    iface[strcspn(iface, "\n")] = '\0';
    if (strlen(iface) == 0) return;

    printf("Enter IP address and subnet in CIDR notation (e.g. 192.168.1.100/24): ");
    fflush(stdout);
    if (fgets(ip_cidr, sizeof(ip_cidr), stdin) == NULL) return;
    ip_cidr[strcspn(ip_cidr, "\n")] = '\0';
    if (strlen(ip_cidr) == 0) return;

    log_info("NETWORK", "Interface configuration command generated (learning mode)");

    printf("\n[Learning Mode] Exact Linux commands to configure interface:\n\n");
    printf("1. Assign IP address:\n");
    printf("   ip addr add %s dev %s\n\n", ip_cidr, iface);
    printf("   Explanation:\n");
    printf("   - ip: The modern Linux utility for network routing, device, and address configuration.\n");
    printf("   - addr add: Command to bind a protocol address to an interface.\n");
    printf("   - %s: The address to assign. The '/24' (or other value) defines the prefix length of the network mask.\n", ip_cidr);
    printf("   - dev %s: Specifies the network device to which the address will be bound.\n\n", iface);

    printf("2. Enable the link:\n");
    printf("   ip link set %s up\n\n", iface);
    printf("   Explanation:\n");
    printf("   - ip link set: Modifies device configuration parameters in the kernel.\n");
    printf("   - %s: The network interface name to modify.\n", iface);
    printf("   - up: Set the device status to operational/active.\n");
}

/* Helper to execute link set state commands */
static int run_ip_link_command(const char* iface, const char* state) {
    char* argv[] = {"ip", "link", "set", (char*)iface, (char*)state, NULL};
    int exit_code = 0;
    
    int res = run_command_direct(NULL, "ip", argv, &exit_code);
    if (res != 0) {
        printf("Error: Failed to execute system process: %s\n", strerror(errno));
        return -1;
    }
    
    if (exit_code != 0) {
        printf("Error: Command execution failed (exit status %d).\n", exit_code);
        if (exit_code == 1 || exit_code == 255) {
            printf("Permission Denied: Root privileges are required to change interface link status.\n");
            printf("Try running with sudo, or execute: sudo ip link set %s %s\n", iface, state);
        }
        return -1;
    }
    
    printf("Successfully set interface %s link state to %s.\n", iface, state);
    return 0;
}

/* 4 & 5. Bring Interface UP/DOWN */
void network_bring_interface_state(const char* state) {
    char iface[64];
    char mode_buf[128];

    printf("Enter interface name: ");
    fflush(stdout);
    if (fgets(iface, sizeof(iface), stdin) == NULL) return;
    iface[strcspn(iface, "\n")] = '\0';
    if (strlen(iface) == 0) return;

    extern int is_interactive;
    int choice = 0;
    if (is_interactive) {
        const char* mode_options[] = {
            "Learning Mode (Chỉ hiển thị lệnh cấu hình)",
            "Apply for real (Áp dụng thật lên hệ thống)"
        };
        int sel = ui_select_menu("Choose action mode", mode_options, 2);
        if (sel == 0) choice = 1;
        else if (sel == 1) choice = 2;
        else return;
    } else {
        printf("\nChoose action mode:\n");
        printf("1. Learning Mode\n");
        printf("2. Apply for real\n");
        printf("Select option: ");
        fflush(stdout);
        if (fgets(mode_buf, sizeof(mode_buf), stdin) == NULL) return;
        choice = atoi(mode_buf);
    }

    if (choice == 1) {
        log_info("NETWORK", "Interface %s link state command shown (learning mode)", state);
        printf("\n[Learning Mode] Command to bring interface %s:\n", state);
        printf("ip link set %s %s\n", iface, state);
    } else if (choice == 2) {
        log_info("NETWORK", "Applying interface %s link state UP/DOWN", state);
        run_ip_link_command(iface, state);
    } else {
        printf("Invalid choice.\n");
    }
}

/* 6. Show Routing Table */
int network_show_routes_ip(void) {
    char* argv[] = {"ip", "route", NULL};
    int exit_code = 0;
    log_info("NETWORK", "Routing table requested");
    printf("\n--- Routing Table (ip route) ---\n");
    int res = run_command_direct(NULL, "ip", argv, &exit_code);
    if (res != 0 || exit_code != 0) {
        printf("Error: Failed to fetch routing table via 'ip route'.\n");
        printf("Attempting fallback to legacy /proc/net/route parsing...\n");
        return network_show_routes();
    }
    return 0;
}

/* 7. Ping Host */
void network_ping_host(void) {
    char host[256];
    char count_str[32];
    int count = 4;

    printf("Enter host or IP to ping (e.g. 8.8.8.8): ");
    fflush(stdout);
    if (fgets(host, sizeof(host), stdin) == NULL) return;
    host[strcspn(host, "\n")] = '\0';
    if (strlen(host) == 0) return;

    printf("Enter number of packets to send [%d]: ", count);
    fflush(stdout);
    if (fgets(count_str, sizeof(count_str), stdin) != NULL) {
        count_str[strcspn(count_str, "\n")] = '\0';
        if (strlen(count_str) > 0) {
            count = atoi(count_str);
            if (count <= 0) count = 4;
        }
    }

    log_info("NETWORK", "Pinging host %s (packets: %d)", host, count);

    char count_param[32];
    snprintf(count_param, sizeof(count_param), "%d", count);

    printf("\n--- Pinging host %s with %d packets ---\n", host, count);
    fflush(stdout);

    char* argv[] = {"ping", "-c", count_param, host, NULL};
    int exit_code = 0;
    int res = run_command_direct(NULL, "ping", argv, &exit_code);

    if (res != 0 || exit_code != 0) {
        printf("\nError: Ping command failed or host was unreachable.\n");
    } else {
        printf("\nPing execution completed successfully.\n");
    }
}

/* 8. DNS Lookup */
int network_dns_lookup(void) {
    char hostname[256];
    printf("Enter hostname to resolve (e.g. google.com): ");
    fflush(stdout);
    if (fgets(hostname, sizeof(hostname), stdin) == NULL) return -1;
    hostname[strcspn(hostname, "\n")] = '\0';
    if (strlen(hostname) == 0) return -1;

    log_info("NETWORK", "DNS lookup for %s", hostname);

    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; /* Retrieve both IPv4 and IPv6 */
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(hostname, NULL, &hints, &res);
    if (status != 0) {
        log_error("NETWORK", "DNS resolution failed for %s: %s", hostname, gai_strerror(status));
        printf("Error: DNS resolution failed: %s\n", gai_strerror(status));
        return -1;
    }

    printf("\nResolved addresses for %s:\n", hostname);
    int ipv4_found = 0;
    int ipv6_found = 0;

    for (p = res; p != NULL; p = p->ai_next) {
        char ip_str[INET6_ADDRSTRLEN];
        void* addr;
        const char* type;

        if (p->ai_family == AF_INET) {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
            addr = &(ipv4->sin_addr);
            type = "IPv4";
            ipv4_found = 1;
        } else if (p->ai_family == AF_INET6) {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            type = "IPv6";
            ipv6_found = 1;
        } else {
            continue;
        }

        if (inet_ntop(p->ai_family, addr, ip_str, sizeof(ip_str)) != NULL) {
            printf("- %s: %s\n", type, ip_str);
        }
    }

    freeaddrinfo(res);

    if (!ipv4_found && !ipv6_found) {
        printf("No IP addresses resolved.\n");
    }

    return 0;
}

/* 9. Socket Statistics */
int network_socket_statistics(void) {
    char* argv[] = {"ss", "-tulnp", NULL};
    int exit_code = 0;
    log_info("NETWORK", "Socket statistics requested");
    printf("\n--- Active TCP/UDP Sockets (ss -tulnp) ---\n");
    printf("Note: Process ownership (PID) information requires root permissions (sudo).\n\n");
    
    int res = run_command_direct(NULL, "ss", argv, &exit_code);
    if (res != 0 || exit_code != 0) {
        printf("Error: Failed to execute 'ss -tulnp'. Fallback to 'netstat -tulnp'...\n");
        char* netstat_argv[] = {"netstat", "-tulnp", NULL};
        res = run_command_direct(NULL, "netstat", netstat_argv, &exit_code);
        if (res != 0 || exit_code != 0) {
            printf("Error: Failed to retrieve socket statistics.\n");
            return -1;
        }
    }
    return 0;
}

/* Menu Loop */
void network_mgr_run(void) {
    int choice;
    log_info("NETWORK", "Entering Network Manager");

    while (1) {
        printf("\n========================================\n");
        printf("Network Manager\n");
        printf("========================================\n");
        printf("1. List Network Interfaces\n");
        printf("2. Interface Information\n");
        printf("3. Configure Interface (Learning Mode)\n");
        printf("4. Bring Interface UP\n");
        printf("5. Bring Interface DOWN\n");
        printf("6. Show Routing Table\n");
        printf("7. Ping Host\n");
        printf("8. DNS Lookup\n");
        printf("9. Socket Statistics\n");
        printf("0. Return\n");
        printf("========================================\n");
        printf("Select option: ");
        fflush(stdout);

        choice = read_network_choice();

        if (choice < 0) {
            continue;
        }

        if (choice > 9) {
            printf("\nInvalid input. Please choose a number between 0 and 9.\n");
            network_menu_pause();
            continue;
        }

        if (choice == 0) {
            log_info("NETWORK", "Leaving Network Manager");
            break;
        }

        if (choice == 1) {
            network_list_interfaces_clean();
            network_menu_pause();
        } else if (choice == 2) {
            network_interface_info();
            network_menu_pause();
        } else if (choice == 3) {
            network_configure_interface_learning();
            network_menu_pause();
        } else if (choice == 4) {
            network_bring_interface_state("up");
            network_menu_pause();
        } else if (choice == 5) {
            network_bring_interface_state("down");
            network_menu_pause();
        } else if (choice == 6) {
            network_show_routes_ip();
            network_menu_pause();
        } else if (choice == 7) {
            network_ping_host();
            network_menu_pause();
        } else if (choice == 8) {
            network_dns_lookup();
            network_menu_pause();
        } else if (choice == 9) {
            network_socket_statistics();
            network_menu_pause();
        }
    }
}

/* Original functions below to guarantee that tests continue to compile/pass */

int network_list_interfaces(void) {
    struct ifaddrs *ifaddr, *ifa;
    char processed[64][IFNAMSIZ];
    int processed_count = 0;

    log_info("NETWORK", "Interface enumeration started (compatibility mode)");

    if (getifaddrs(&ifaddr) == -1) {
        log_error("NETWORK", "API failure: getifaddrs() failed (errno %d)", errno);
        printf("Error: Failed to retrieve network interfaces.\n");
        return -1;
    }

    printf("\nDiscovered Network Interfaces:\n");

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_name == NULL) {
            continue;
        }

        /* Check for duplicates */
        int already_processed = 0;
        for (int i = 0; i < processed_count; ++i) {
            if (strcmp(processed[i], ifa->ifa_name) == 0) {
                already_processed = 1;
                break;
            }
        }
        if (already_processed) {
            continue;
        }

        /* Register as processed */
        if (processed_count < 64) {
            strncpy(processed[processed_count], ifa->ifa_name, IFNAMSIZ - 1);
            processed[processed_count][IFNAMSIZ - 1] = '\0';
            processed_count++;
        }

        unsigned int flags = ifa->ifa_flags;
        const char* status = (flags & IFF_UP) ? "UP" : "DOWN";

        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        int mtu = -1;
        char mac_str[32] = "N/A";
        sa_family_t hw_family = 0;

        if (sock >= 0) {
            struct ifreq ifr;
            strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);
            ifr.ifr_name[IFNAMSIZ - 1] = '\0';

            if (ioctl(sock, SIOCGIFMTU, &ifr) == 0) {
                mtu = ifr.ifr_mtu;
            }

            if (!(flags & IFF_LOOPBACK)) {
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    hw_family = ifr.ifr_hwaddr.sa_family;
                    unsigned char* mac = (unsigned char*)ifr.ifr_hwaddr.sa_data;
                    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                }
            }
            close(sock);
        }

        const char* type = "Unknown";
        if (flags & IFF_LOOPBACK) {
            type = "Loopback";
        } else if (hw_family == ARPHRD_ETHER) {
            if (strncmp(ifa->ifa_name, "wl", 2) == 0 || strncmp(ifa->ifa_name, "wlan", 4) == 0) {
                type = "Wireless";
            } else if (strncmp(ifa->ifa_name, "docker", 6) == 0 || strncmp(ifa->ifa_name, "veth", 4) == 0 || strncmp(ifa->ifa_name, "br", 2) == 0) {
                type = "Virtual/Bridge";
            } else {
                type = "Ethernet";
            }
        }

        printf("--------------------------------------------------\n");
        printf("Interface Name: %s\n", ifa->ifa_name);
        printf("Type:           %s\n", type);
        printf("Status:         %s\n", status);
        if (mtu >= 0) {
            printf("MTU:            %d\n", mtu);
        } else {
            printf("MTU:            Unknown\n");
        }
        printf("MAC Address:    %s\n", mac_str);

        struct ifaddrs *search_ifa;
        for (search_ifa = ifaddr; search_ifa != NULL; search_ifa = search_ifa->ifa_next) {
            if (strcmp(search_ifa->ifa_name, ifa->ifa_name) != 0 || search_ifa->ifa_addr == NULL) {
                continue;
            }

            int family = search_ifa->ifa_addr->sa_family;
            if (family == AF_INET) {
                char ip_str[INET_ADDRSTRLEN];
                struct sockaddr_in* addr = (struct sockaddr_in*)search_ifa->ifa_addr;
                if (inet_ntop(AF_INET, &addr->sin_addr, ip_str, sizeof(ip_str)) != NULL) {
                    printf("IPv4 Address:   %s\n", ip_str);
                }
            } else if (family == AF_INET6) {
                char ip_str[INET6_ADDRSTRLEN];
                struct sockaddr_in6* addr = (struct sockaddr_in6*)search_ifa->ifa_addr;
                if (inet_ntop(AF_INET6, &addr->sin6_addr, ip_str, sizeof(ip_str)) != NULL) {
                    printf("IPv6 Address:   %s\n", ip_str);
                }
            }
        }
    }

    printf("--------------------------------------------------\n");
    freeifaddrs(ifaddr);
    log_info("NETWORK", "Interfaces discovered");
    return 0;
}

int network_show_routes(void) {
    log_info("NETWORK", "Routing inspection started (proc-based)");

    int fd = open("/proc/net/route", O_RDONLY);
    if (fd < 0) {
        log_error("NETWORK", "API failure: open() failed on /proc/net/route (errno %d)", errno);
        printf("Error: Failed to open routing table file.\n");
        return -1;
    }

    char buf[8192];
    ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (bytes_read < 0) {
        log_error("NETWORK", "API failure: read() failed on /proc/net/route (errno %d)", errno);
        printf("Error: Failed to read routing table.\n");
        return -1;
    }
    buf[bytes_read] = '\0';

    return parse_routing_table(buf);
}

int parse_routing_table(const char* buf) {
    typedef struct {
        char iface[32];
        char dest[INET_ADDRSTRLEN];
        char gw[INET_ADDRSTRLEN];
        char mask[INET_ADDRSTRLEN];
        char flags[8];
    } route_entry_t;

    route_entry_t routes[64];
    int route_count = 0;
    char gw_ip[INET_ADDRSTRLEN] = "None";
    int gw_found = 0;

    char* buf_copy = strdup(buf);
    if (!buf_copy) {
        log_error("NETWORK", "Memory allocation failed for strdup in parse_routing_table");
        return -1;
    }

    char* line = buf_copy;
    char* next_line;

    if ((line = strchr(line, '\n')) != NULL) {
        line++;
    }

    while (line && *line != '\0') {
        next_line = strchr(line, '\n');
        if (next_line) {
            *next_line = '\0';
            next_line++;
        }

        char iface[32], dest_hex[32], gw_hex[32], mask_hex[32];
        unsigned int flags_val;
        if (sscanf(line, "%31s %31s %31s %x %*d %*d %*d %31s", iface, dest_hex, gw_hex, &flags_val, mask_hex) == 5) {
            if (route_count < 64) {
                route_entry_t* r = &routes[route_count];
                strncpy(r->iface, iface, sizeof(r->iface) - 1);
                r->iface[sizeof(r->iface) - 1] = '\0';

                hex_to_ip(dest_hex, r->dest);
                hex_to_ip(gw_hex, r->gw);
                hex_to_ip(mask_hex, r->mask);

                r->flags[0] = '\0';
                if (flags_val & 0x0001) strcat(r->flags, "U");
                if (flags_val & 0x0002) strcat(r->flags, "G");
                if (flags_val & 0x0004) strcat(r->flags, "H");

                if (strcmp(dest_hex, "00000000") == 0 && strcmp(gw_hex, "00000000") != 0 && !gw_found) {
                    strncpy(gw_ip, r->gw, sizeof(gw_ip) - 1);
                    gw_found = 1;
                    log_info("NETWORK", "Default gateway detected");
                }

                route_count++;
            }
        }
        line = next_line;
    }

    printf("\nDefault Gateway: %s\n\n", gw_found ? gw_ip : "None");

    printf("%-10s %-16s %-16s %-16s %-6s\n", "Interface", "Destination", "Gateway", "Netmask", "Flags");
    printf("----------------------------------------------------------------------\n");
    for (int i = 0; i < route_count; i++) {
        printf("%-10s %-16s %-16s %-16s %-6s\n",
               routes[i].iface,
               routes[i].dest,
               routes[i].gw,
               routes[i].mask,
               routes[i].flags);
    }
    printf("----------------------------------------------------------------------\n");

    free(buf_copy);
    log_info("NETWORK", "Route entries parsed");
    return 0;
}

int network_show_host_info(void) {
    char hostname[256] = "N/A";
    char short_hostname[256] = "N/A";
    char fqdn[256] = "N/A";
    char domain[256] = "N/A";
    char node_name[256] = "N/A";

    log_info("NETWORK", "Hostname queried");

    if (gethostname(hostname, sizeof(hostname)) != 0) {
        log_error("NETWORK", "API failure: gethostname() failed (errno %d)", errno);
        printf("Error: Failed to query hostname.\n");
        return -1;
    }

    strncpy(short_hostname, hostname, sizeof(short_hostname) - 1);
    short_hostname[sizeof(short_hostname) - 1] = '\0';
    char* dot = strchr(short_hostname, '.');
    if (dot) {
        *dot = '\0';
    }

    struct addrinfo hints, *info = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    if (getaddrinfo(hostname, NULL, &hints, &info) == 0) {
        if (info && info->ai_canonname) {
            strncpy(fqdn, info->ai_canonname, sizeof(fqdn) - 1);
            fqdn[sizeof(fqdn) - 1] = '\0';
            log_info("NETWORK", "FQDN resolved");
        } else {
            strncpy(fqdn, hostname, sizeof(fqdn) - 1);
            fqdn[sizeof(fqdn) - 1] = '\0';
        }
        if (info) {
            freeaddrinfo(info);
        }
    } else {
        log_error("NETWORK", "getaddrinfo failed to resolve FQDN for %s (errno %d)", hostname, errno);
        strncpy(fqdn, hostname, sizeof(fqdn) - 1);
        fqdn[sizeof(fqdn) - 1] = '\0';
    }

    char* first_dot = strchr(fqdn, '.');
    if (first_dot && *(first_dot + 1) != '\0') {
        strncpy(domain, first_dot + 1, sizeof(domain) - 1);
        domain[sizeof(domain) - 1] = '\0';
    }

    struct utsname uts;
    if (uname(&uts) == 0) {
        strncpy(node_name, uts.nodename, sizeof(node_name) - 1);
        node_name[sizeof(node_name) - 1] = '\0';
    } else {
        log_error("NETWORK", "API failure: uname() failed (errno %d)", errno);
    }

    printf("\n=== Host Network Identity ===\n");
    printf("Hostname:           %s\n", hostname);
    printf("Short Hostname:     %s\n", short_hostname);
    printf("FQDN:               %s\n", fqdn);
    printf("Domain Name:        %s\n", domain);
    printf("Kernel Node Name:   %s\n", node_name);
    printf("=============================\n");

    return 0;
}
