/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: modules/network/network_mgr.c
 * Purpose: Network interface, routing, and host configuration Diagnostics.
 */

#define _GNU_SOURCE /* Required for strdup */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <net/if_arp.h>
#include "network_mgr.h"
#include "logger.h"

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

/* Static submenu helpers */
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

static void network_menu_pause(void) {
    int c;
    printf("\nPress ENTER to continue...");
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF);
}

void network_mgr_run(void) {
    int choice;
    log_info("NETWORK", "Entering Network Manager");

    while (1) {
        printf("\n========================================\n");
        printf("            Network Manager\n");
        printf("========================================\n");
        printf("1. Host Information\n");
        printf("2. Interface Information\n");
        printf("3. Routing Information\n");
        printf("4. DNS Information (Future)\n");
        printf("5. Traffic Statistics (Future)\n");
        printf("0. Return\n");
        printf("========================================\n");
        printf("Select option: ");
        fflush(stdout);

        choice = read_network_choice();

        if (choice < 0) {
            continue;
        }

        if (choice > 5) {
            printf("\nInvalid input. Please choose a number between 0 and 5.\n");
            network_menu_pause();
            continue;
        }

        if (choice == 0) {
            log_info("NETWORK", "Leaving Network Manager");
            break;
        }

        if (choice == 4 || choice == 5) {
            printf("\nComing in a future sprint.\n");
            network_menu_pause();
            continue;
        }

        if (choice == 1) {
            network_show_host_info();
            network_menu_pause();
        } else if (choice == 2) {
            network_list_interfaces();
            network_menu_pause();
        } else if (choice == 3) {
            network_show_routes();
            network_menu_pause();
        }
    }
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

    /* Extract Short Hostname */
    strncpy(short_hostname, hostname, sizeof(short_hostname) - 1);
    short_hostname[sizeof(short_hostname) - 1] = '\0';
    char* dot = strchr(short_hostname, '.');
    if (dot) {
        *dot = '\0';
    }

    /* FQDN resolution using getaddrinfo */
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

    /* Extract Domain Name */
    char* first_dot = strchr(fqdn, '.');
    if (first_dot && *(first_dot + 1) != '\0') {
        strncpy(domain, first_dot + 1, sizeof(domain) - 1);
        domain[sizeof(domain) - 1] = '\0';
    }

    /* Kernel Node Name */
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

int network_list_interfaces(void) {
    struct ifaddrs *ifaddr, *ifa;
    char processed[64][IFNAMSIZ];
    int processed_count = 0;

    log_info("NETWORK", "Interface enumeration started");

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

        /* Retrieve interface configuration via flags and socket ioctl */
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

            /* Query MTU */
            if (ioctl(sock, SIOCGIFMTU, &ifr) == 0) {
                mtu = ifr.ifr_mtu;
            }

            /* Query MAC hardware address (if not a loopback interface) */
            if (!(flags & IFF_LOOPBACK)) {
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    hw_family = ifr.ifr_hwaddr.sa_family;
                    unsigned char* mac = (unsigned char*)ifr.ifr_hwaddr.sa_data;
                    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                }
            }
            close(sock);
        } else {
            log_error("NETWORK", "API failure: socket() failed for ioctls on interface %s (errno %d)", ifa->ifa_name, errno);
        }

        /* Determine interface type */
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
        } else {
            /* Fallback type matching based on name patterns */
            if (strncmp(ifa->ifa_name, "lo", 2) == 0) {
                type = "Loopback";
            } else if (strncmp(ifa->ifa_name, "eth", 3) == 0 || strncmp(ifa->ifa_name, "en", 2) == 0) {
                type = "Ethernet";
            } else if (strncmp(ifa->ifa_name, "wl", 2) == 0 || strncmp(ifa->ifa_name, "wlan", 4) == 0) {
                type = "Wireless";
            } else if (strncmp(ifa->ifa_name, "docker", 6) == 0 || strncmp(ifa->ifa_name, "veth", 4) == 0 || strncmp(ifa->ifa_name, "br", 2) == 0) {
                type = "Virtual/Bridge";
            }
        }

        /* Print interface details */
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

        /* Print IPv4 and IPv6 addresses configured on this interface */
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
    log_info("NETWORK", "Routing inspection started");

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

    int res = parse_routing_table(buf);
    return res;
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

                /* Format flags description */
                r->flags[0] = '\0';
                if (flags_val & 0x0001) strcat(r->flags, "U"); /* RTF_UP */
                if (flags_val & 0x0002) strcat(r->flags, "G"); /* RTF_GATEWAY */
                if (flags_val & 0x0004) strcat(r->flags, "H"); /* RTF_HOST */

                /* Detect gateway (Destination = 0.0.0.0 and Gateway != 0.0.0.0) */
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
