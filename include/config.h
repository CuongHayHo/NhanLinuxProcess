/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/config.h
 * Purpose: Central configurations, constraints, default paths, and ports.
 */

#ifndef CONFIG_H
#define CONFIG_H

/* Application Identification */
#define APP_NAME            "Linux System Manager"
#define APP_VERSION         "0.7.0-shell-complete"

/* Default Paths */
#define CONFIG_FILE_PATH    "config/sysmgr.conf"
#define DEFAULT_TMP_DIR     "tmp/"
#define PROC_NET_ROUTE_PATH "/proc/net/route"
#define PROC_NET_DEV_PATH   "/proc/net/dev"
#define ETC_RESOLV_CONF_PATH "/etc/resolv.conf"

/* Buffer Sizes */
#define CHAT_BUFFER_SIZE    512
#define MAX_USERNAME_LEN    50
#define MAX_CLIENTS         100
#define DEFAULT_BUF_SIZE    4096
#define COMPACT_BUF_SIZE    256

/* Default Ports and IPs */
#define DEFAULT_PORT        8080
#define DEFAULT_IP          "127.0.0.1"

/* Log Locations */
#define LOG_PATH_SYSTEM     "logs/system.log"
#define LOG_PATH_NETWORK    "logs/network.log"
#define LOG_PATH_PROCESS    "logs/process.log"
#define LOG_PATH_KERNEL     "logs/kernel.log"

#endif /* CONFIG_H */
