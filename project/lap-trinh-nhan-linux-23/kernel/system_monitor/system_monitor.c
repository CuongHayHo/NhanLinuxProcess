/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: kernel/system_monitor/system_monitor.c
 * Purpose: Simple system monitor Linux Kernel Module (LKM).
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux System Manager Team");
MODULE_DESCRIPTION("Simple System Monitor LKM for University Reference Project");

static int log_level = 1;
module_param(log_level, int, 0644);
MODULE_PARM_DESC(log_level, "Kernel Monitor log level verbosity (1-3)");

static char* target_process = "";
module_param(target_process, charp, 0644);
MODULE_PARM_DESC(target_process, "Target process name to filter monitor statistics");

static int __init monitor_init(void) {
    printk(KERN_INFO "system_monitor: LKM loaded successfully.\n");
    printk(KERN_INFO "system_monitor: log_level = %d, target_process = '%s'\n", log_level, target_process);
    /* TODO: Implement process traversal and memory metric printing to dmesg */
    return 0;
}

static void __exit monitor_exit(void) {
    printk(KERN_INFO "system_monitor: LKM unloaded successfully.\n");
}

module_init(monitor_init);
module_exit(monitor_exit);
