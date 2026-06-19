/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/config.h
 * Purpose: Central configurations and constraints.
 */

#ifndef CONFIG_H
#define CONFIG_H

#define APP_NAME            "Linux System Manager"
#define APP_VERSION         "1.0.0"

#define DEFAULT_PORT        9099
#define DEFAULT_IP          "127.0.0.1"

#define CHAT_BUFFER_SIZE    512
#define MAX_USERNAME_LEN    50
#define MAX_CLIENTS         100

#define LOG_PATH_SYSTEM     "logs/system.log"
#define LOG_PATH_NETWORK    "logs/network.log"
#define LOG_PATH_PROCESS    "logs/process.log"
#define LOG_PATH_KERNEL     "logs/kernel.log"

#define CONFIG_FILE_PATH    "config/sysmgr.conf"

#endif /* CONFIG_H */
