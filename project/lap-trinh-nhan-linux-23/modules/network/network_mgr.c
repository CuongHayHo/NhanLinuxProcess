/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/network/network_mgr.c
 * Purpose: Network interface and gateway routing metrics collector.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include "network_mgr.h"
#include "logger.h"

/* Static helper to query MAC address via socket/ioctl */
static int get_mac_address(const char* if_name, char* mac_out) {
    int sock;
    struct ifreq ifr;
    
    mac_out[0] = '\0';
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        log_error("NETWORK", "API failure: socket() failed to query MAC for %s (errno %d)", if_name, errno);
        return -1;
    }
    
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) != 0) {
        /* Virtual interfaces or loopback may not support SIOCGIFHWADDR */
        close(sock);
        return -1;
    }
    
    close(sock);
    
    unsigned char* mac = (unsigned char*)ifr.ifr_hwaddr.sa_data;
    sprintf(mac_out, "%02x:%02x:%02x:%02x:%02x:%02x",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    return 0;
}

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

void network_mgr_run(void) {
    log_info("NETWORK", "Network information requested");
    
    printf("\n=== Network Information Panel ===\n");
    network_show_hostname();
    network_show_gateway();
    network_show_dns();
    network_list_interfaces();
    network_show_routes();
    network_show_statistics();
    printf("=================================\n");
}

int network_mgr_get_interfaces(void) {
    log_info("NETWORK", "network_mgr_get_interfaces called");
    return network_list_interfaces();
}

int network_mgr_get_routing_table(void) {
    log_info("NETWORK", "network_mgr_get_routing_table called");
    return network_show_routes();
}

int network_mgr_get_dns_servers(char dns_out[][64], int max_dns) {
    (void)dns_out;
    (void)max_dns;
    log_info("NETWORK", "network_mgr_get_dns_servers called");
    return network_show_dns();
}

int network_list_interfaces(void) {
    struct ifaddrs *ifaddr, *ifa;
    char ipv4_str[INET_ADDRSTRLEN];
    char ipv6_str[INET6_ADDRSTRLEN];
    char mac_str[32];
    char printed_ifs[64][32];
    int printed_count = 0;

    if (getifaddrs(&ifaddr) == -1) {
        log_error("NETWORK", "API failure: getifaddrs() failed (errno %d)", errno);
        printf("Error: Failed to retrieve network interfaces.\n");
        return -1;
    }

    printf("\n%-10s %-8s %-18s %-16s %-16s\n", "Interface", "Status", "MAC Address", "IPv4 Address", "IPv6 Address");
    printf("------------------------------------------------------------------------------------\n");

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        /* De-duplicate interface name listings */
        int already_printed = 0;
        for (int i = 0; i < printed_count; i++) {
            if (strcmp(printed_ifs[i], ifa->ifa_name) == 0) {
                already_printed = 1;
                break;
            }
        }
        if (already_printed) continue;

        if (printed_count < 64) {
            strncpy(printed_ifs[printed_count], ifa->ifa_name, 31);
            printed_ifs[printed_count][31] = '\0';
            printed_count++;
        }

        log_info("NETWORK", "Interface discovered: %s", ifa->ifa_name);

        /* Extract IP addresses for this interface */
        ipv4_str[0] = '\0';
        ipv6_str[0] = '\0';
        
        struct ifaddrs* search_ifa;
        for (search_ifa = ifaddr; search_ifa != NULL; search_ifa = search_ifa->ifa_next) {
            if (strcmp(search_ifa->ifa_name, ifa->ifa_name) != 0 || search_ifa->ifa_addr == NULL) {
                continue;
            }
            int family = search_ifa->ifa_addr->sa_family;
            if (family == AF_INET) {
                struct sockaddr_in* addr = (struct sockaddr_in*)search_ifa->ifa_addr;
                if (inet_ntop(AF_INET, &addr->sin_addr, ipv4_str, sizeof(ipv4_str)) == NULL) {
                    log_error("NETWORK", "API failure: inet_ntop() IPv4 failed for %s (errno %d)", ifa->ifa_name, errno);
                }
            } else if (family == AF_INET6) {
                struct sockaddr_in6* addr = (struct sockaddr_in6*)search_ifa->ifa_addr;
                if (inet_ntop(AF_INET6, &addr->sin6_addr, ipv6_str, sizeof(ipv6_str)) == NULL) {
                    log_error("NETWORK", "API failure: inet_ntop() IPv6 failed for %s (errno %d)", ifa->ifa_name, errno);
                }
            }
        }

        /* MAC Address query */
        if (get_mac_address(ifa->ifa_name, mac_str) != 0) {
            strcpy(mac_str, "N/A");
        }

        /* UP/DOWN status */
        const char* status_str = (ifa->ifa_flags & IFF_UP) ? "UP" : "DOWN";

        printf("%-10s %-8s %-18s %-16s %-16s\n", 
               ifa->ifa_name, 
               status_str, 
               mac_str, 
               (strlen(ipv4_str) > 0) ? ipv4_str : "N/A", 
               (strlen(ipv6_str) > 0) ? ipv6_str : "N/A");
    }

    printf("------------------------------------------------------------------------------------\n");
    freeifaddrs(ifaddr);
    return 0;
}

int network_show_hostname(void) {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        log_error("NETWORK", "API failure: gethostname() failed (errno %d)", errno);
        printf("Hostname:          Unknown\n");
        return -1;
    }
    printf("Hostname:          %s\n", hostname);
    return 0;
}

int network_show_ip(void) {
    struct ifaddrs *ifaddr, *ifa;
    char ip_str[INET6_ADDRSTRLEN];
    
    if (getifaddrs(&ifaddr) == -1) {
        log_error("NETWORK", "API failure: getifaddrs() failed (errno %d)", errno);
        return -1;
    }
    
    printf("\nIP Addresses:\n");
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) {
            struct sockaddr_in* addr = (struct sockaddr_in*)ifa->ifa_addr;
            if (inet_ntop(AF_INET, &addr->sin_addr, ip_str, sizeof(ip_str)) != NULL) {
                printf("  %-10s IPv4: %s\n", ifa->ifa_name, ip_str);
            }
        } else if (family == AF_INET6) {
            struct sockaddr_in6* addr = (struct sockaddr_in6*)ifa->ifa_addr;
            if (inet_ntop(AF_INET6, &addr->sin6_addr, ip_str, sizeof(ip_str)) != NULL) {
                printf("  %-10s IPv6: %s\n", ifa->ifa_name, ip_str);
            }
        }
    }
    freeifaddrs(ifaddr);
    return 0;
}

int network_show_mac(void) {
    struct ifaddrs *ifaddr, *ifa;
    char mac_str[32];
    char printed_ifs[64][32];
    int printed_count = 0;
    
    if (getifaddrs(&ifaddr) == -1) {
        log_error("NETWORK", "API failure: getifaddrs() failed (errno %d)", errno);
        return -1;
    }
    
    printf("\nMAC Addresses:\n");
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        int already_printed = 0;
        for (int i = 0; i < printed_count; i++) {
            if (strcmp(printed_ifs[i], ifa->ifa_name) == 0) {
                already_printed = 1;
                break;
            }
        }
        if (already_printed) continue;
        
        if (printed_count < 64) {
            strncpy(printed_ifs[printed_count], ifa->ifa_name, 31);
            printed_ifs[printed_count][31] = '\0';
            printed_count++;
        }
        
        if (get_mac_address(ifa->ifa_name, mac_str) == 0) {
            printf("  %-10s MAC:  %s\n", ifa->ifa_name, mac_str);
        } else {
            printf("  %-10s MAC:  N/A\n", ifa->ifa_name);
        }
    }
    freeifaddrs(ifaddr);
    return 0;
}

static char* read_entire_file(const char* path, size_t* out_size) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }

    size_t capacity = 4096;
    size_t total_read = 0;
    char* buf = malloc(capacity);
    if (!buf) {
        close(fd);
        return NULL;
    }

    while (1) {
        ssize_t n = read(fd, buf + total_read, capacity - total_read - 1);
        if (n < 0) {
            free(buf);
            close(fd);
            return NULL;
        }
        if (n == 0) {
            break;
        }
        total_read += n;
        if (total_read >= capacity - 1) {
            capacity *= 2;
            char* new_buf = realloc(buf, capacity);
            if (!new_buf) {
                free(buf);
                close(fd);
                return NULL;
            }
            buf = new_buf;
        }
    }

    buf[total_read] = '\0';
    close(fd);
    if (out_size) {
        *out_size = total_read;
    }
    return buf;
}

int network_show_gateway(void) {
    size_t size;
    char* buf = read_entire_file("/proc/net/route", &size);
    if (!buf) {
        if (errno == ENOENT) {
            log_error("NETWORK", "Missing file: /proc/net/route not found");
        } else if (errno == EACCES) {
            log_error("NETWORK", "Permission denied: Cannot read /proc/net/route (errno %d)", errno);
        } else {
            log_error("NETWORK", "API failure: open() or read() on /proc/net/route failed (errno %d)", errno);
        }
        printf("Default Gateway:   Unknown\n");
        return -1;
    }

    char gw_ip[INET_ADDRSTRLEN] = "N/A";
    int found = 0;
    char* line = buf;
    char* next_line;

    /* Skip header line */
    if ((line = strchr(line, '\n')) != NULL) {
        line++;
    }

    while (line && *line != '\0') {
        next_line = strchr(line, '\n');
        if (next_line) {
            *next_line = '\0';
            next_line++;
        }

        char iface[32], dest_hex[32], gw_hex[32];
        unsigned int flags_val;
        if (sscanf(line, "%31s %31s %31s %x", iface, dest_hex, gw_hex, &flags_val) == 4) {
            if (strcmp(dest_hex, "00000000") == 0 && strcmp(gw_hex, "00000000") != 0) {
                hex_to_ip(gw_hex, gw_ip);
                found = 1;
                break;
            }
        }
        line = next_line;
    }

    free(buf);
    log_info("NETWORK", "Gateway queried");
    printf("Default Gateway:   %s\n", gw_ip);
    return found ? 0 : -1;
}

int network_show_dns(void) {
    size_t size;
    char* buf = read_entire_file("/etc/resolv.conf", &size);
    if (!buf) {
        if (errno == ENOENT) {
            log_error("NETWORK", "Missing file: /etc/resolv.conf not found");
        } else if (errno == EACCES) {
            log_error("NETWORK", "Permission denied: Cannot read /etc/resolv.conf (errno %d)", errno);
        } else {
            log_error("NETWORK", "API failure: open() or read() on /etc/resolv.conf failed (errno %d)", errno);
        }
        printf("DNS Servers:       None configured\n");
        return -1;
    }

    int dns_count = 0;
    printf("DNS Servers:       ");
    char* line = buf;
    char* next_line;
    while (line && *line != '\0') {
        next_line = strchr(line, '\n');
        if (next_line) {
            *next_line = '\0';
            next_line++;
        }

        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p != '#' && *p != ';' && *p != '\0') {
            char key[32], val[64];
            if (sscanf(p, "%31s %63s", key, val) == 2) {
                if (strcmp(key, "nameserver") == 0) {
                    if (dns_count > 0) {
                        printf(", ");
                    }
                    printf("%s", val);
                    dns_count++;
                }
            }
        }
        line = next_line;
    }

    if (dns_count == 0) {
        printf("None configured");
    }
    printf("\n");

    free(buf);
    log_info("NETWORK", "DNS queried");
    return 0;
}

int network_show_routes(void) {
    size_t size;
    char* buf = read_entire_file("/proc/net/route", &size);
    if (!buf) {
        if (errno == ENOENT) {
            log_error("NETWORK", "Missing file: /proc/net/route not found");
        } else if (errno == EACCES) {
            log_error("NETWORK", "Permission denied: Cannot read /proc/net/route (errno %d)", errno);
        } else {
            log_error("NETWORK", "API failure: open() or read() on /proc/net/route failed (errno %d)", errno);
        }
        printf("Error: Failed to retrieve routing table.\n");
        return -1;
    }

    printf("\nRouting Table:\n");
    printf("%-10s %-16s %-16s %-16s %-6s %-6s\n", "Interface", "Destination", "Gateway", "Genmask", "Flags", "Metric");
    printf("------------------------------------------------------------------------------------\n");

    char* line = buf;
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
        int metric_val;
        
        if (sscanf(line, "%31s %31s %31s %x %*d %*d %d %31s", iface, dest_hex, gw_hex, &flags_val, &metric_val, mask_hex) == 6) {
            char dest_ip[INET_ADDRSTRLEN];
            char gw_ip[INET_ADDRSTRLEN];
            char mask_ip[INET_ADDRSTRLEN];
            char flags_str[8] = "";

            hex_to_ip(dest_hex, dest_ip);
            hex_to_ip(gw_hex, gw_ip);
            hex_to_ip(mask_hex, mask_ip);

            if (flags_val & 0x0001) strcat(flags_str, "U");
            if (flags_val & 0x0002) strcat(flags_str, "G");
            if (flags_val & 0x0004) strcat(flags_str, "H");

            printf("%-10s %-16s %-16s %-16s %-6s %-6d\n", iface, dest_ip, gw_ip, mask_ip, flags_str, metric_val);
        }
        line = next_line;
    }
    printf("------------------------------------------------------------------------------------\n");

    free(buf);
    log_info("NETWORK", "Route table parsed");
    return 0;
}

int network_show_statistics(void) {
    size_t size;
    char* buf = read_entire_file("/proc/net/dev", &size);
    if (!buf) {
        if (errno == ENOENT) {
            log_error("NETWORK", "Missing file: /proc/net/dev not found");
        } else if (errno == EACCES) {
            log_error("NETWORK", "Permission denied: Cannot read /proc/net/dev (errno %d)", errno);
        } else {
            log_error("NETWORK", "API failure: open() or read() on /proc/net/dev failed (errno %d)", errno);
        }
        printf("Error: Failed to retrieve network statistics.\n");
        return -1;
    }

    printf("\nInterface Statistics:\n");
    printf("%-10s %-16s %-12s %-16s %-12s\n", "Interface", "RX Bytes", "RX Packets", "TX Bytes", "TX Packets");
    printf("------------------------------------------------------------------------------------\n");

    char* line = buf;
    char* next_line;
    if ((line = strchr(line, '\n')) != NULL) {
        line++;
        if ((line = strchr(line, '\n')) != NULL) {
            line++;
        }
    }

    while (line && *line != '\0') {
        next_line = strchr(line, '\n');
        if (next_line) {
            *next_line = '\0';
            next_line++;
        }

        char* colon = strchr(line, ':');
        if (colon) {
            char iface[32];
            *colon = '\0';
            
            char* iface_start = line;
            while (*iface_start == ' ' || *iface_start == '\t') {
                iface_start++;
            }
            strncpy(iface, iface_start, sizeof(iface) - 1);
            iface[sizeof(iface) - 1] = '\0';

            unsigned long rx_bytes, rx_packets, tx_bytes, tx_packets;
            if (sscanf(colon + 1, "%lu %lu %*u %*u %*u %*u %*u %*u %lu %lu", 
                       &rx_bytes, &rx_packets, &tx_bytes, &tx_packets) == 4) {
                printf("%-10s %-16lu %-12lu %-16lu %-12lu\n", iface, rx_bytes, rx_packets, tx_bytes, tx_packets);
            }
        }
        line = next_line;
    }
    printf("------------------------------------------------------------------------------------\n");

    free(buf);
    log_info("NETWORK", "Network statistics requested");
    return 0;
}
