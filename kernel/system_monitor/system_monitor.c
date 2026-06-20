/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: kernel/system_monitor/system_monitor.c
 * Purpose: Simple system monitor Linux Kernel Module (LKM) with read-only /proc interface.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/utsname.h>
#include <linux/time.h>
#include <linux/timekeeping.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux System Manager Team");
MODULE_DESCRIPTION("Simple System Monitor LKM with read-only /proc/sysmgr interface");
MODULE_VERSION("1.0");

static int log_level = 1;
module_param(log_level, int, 0644);
MODULE_PARM_DESC(log_level, "Kernel Monitor log level verbosity (1-3)");

static char* target_process = "";
module_param(target_process, charp, 0644);
MODULE_PARM_DESC(target_process, "Target process name to filter monitor statistics");

static time64_t load_time_sec;

// seq_file show callback
static int sysmgr_proc_show(struct seq_file *m, void *v) {
    struct tm tm;
    (void)v;
    time64_to_tm(load_time_sec, 0, &tm);

    seq_printf(m, "Module Name:     %s\n", KBUILD_MODNAME);
    seq_printf(m, "Version:         %s\n", "1.0");
    seq_printf(m, "Kernel Version:  %s\n", init_utsname()->release);
    seq_printf(m, "Load Time:       %04ld-%02d-%02d %02d:%02d:%02d UTC\n",
               (long)(tm.tm_year + 1900), tm.tm_mon + 1, tm.tm_mday,
               tm.tm_hour, tm.tm_min, tm.tm_sec);
    seq_printf(m, "Current Jiffies: %lu\n", jiffies);
    return 0;
}

// open callback
static int sysmgr_proc_open(struct inode *inode, struct file *file) {
    (void)inode;
    return single_open(file, sysmgr_proc_show, NULL);
}

// File operations structure for proc file using proc_ops (Linux 5.6+)
static const struct proc_ops sysmgr_proc_ops = {
    .proc_open    = sysmgr_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static int __init monitor_init(void) {
    struct proc_dir_entry *entry;
    
    load_time_sec = ktime_get_real_seconds();
    printk(KERN_INFO "system_monitor: Module loaded\n");
    printk(KERN_INFO "system_monitor: log_level = %d, target_process = '%s'\n", log_level, target_process);

    // Create /proc/sysmgr
    entry = proc_create("sysmgr", 0444, NULL, &sysmgr_proc_ops);
    if (!entry) {
        printk(KERN_ERR "system_monitor: Errors: failed to create /proc/sysmgr\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "system_monitor: /proc created\n");

    return 0;
}

static void __exit monitor_exit(void) {
    // Remove /proc/sysmgr
    remove_proc_entry("sysmgr", NULL);
    printk(KERN_INFO "system_monitor: /proc removed\n");
    printk(KERN_INFO "system_monitor: Module unloaded\n");
}

module_init(monitor_init);
module_exit(monitor_exit);
