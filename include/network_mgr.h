/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/network_mgr.h
 * Purpose: Network interface diagnostics public API.
 */

#ifndef NETWORK_MGR_H
#define NETWORK_MGR_H

/**
 * Runs the interactive Network Manager sub-menu.
 */
void network_mgr_run(void);

/**
 * Lists all network interfaces and displays their details.
 * Returns 0 on success, -1 on failure.
 */
int network_list_interfaces(void);

/**
 * Parses and displays routing table entries and default gateway.
 * Returns 0 on success, -1 on failure.
 */
int network_show_routes(void);

/**
 * Helper parser to parse routing table content from a buffer.
 * Exposed for testing purposes.
 */
int parse_routing_table(const char* buf);

/**
 * Displays the host network identity parameters (Hostname, FQDN, Domain, Node Name).
 * Returns 0 on success, -1 on failure.
 */
int network_show_host_info(void);

/* REPL command palette entry points */
int network_list_interfaces_clean(void);
int network_interface_info(void);
void network_configure_interface_learning(void);
void network_bring_interface_state(const char* state);
int network_show_routes_ip(void);
void network_ping_host(void);
int network_dns_lookup(void);
int network_socket_statistics(void);

#endif /* NETWORK_MGR_H */
