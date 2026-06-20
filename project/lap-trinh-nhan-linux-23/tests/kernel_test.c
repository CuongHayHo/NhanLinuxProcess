/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/kernel_test.c
 * Purpose: Complete Diagnostic tests and demonstrations for Kernel Manager.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "kernel_mgr.h"
#include "logger.h"

/* Dummy definitions for CLI and TUI subsystems to satisfy links in kernel_mgr.o */
int is_interactive = 0;
int ui_select_menu(const char* title, const char* options[], int options_count) {
    (void)title; (void)options; (void)options_count;
    return -1;
}
void print_prompt_explanation(const char* prompt_msg) {
    (void)prompt_msg;
}

int main(void) {
    printf("Starting Kernel Module Manager Automated Test Suite...\n\n");

    // 1. Initial State / Status check
    printf("[TEST] Checking initial status detection...\n");
    int initial_loaded = is_module_loaded() || is_module_loaded_via_lsmod();
    printf("Initial status: Module loaded = %s\n", initial_loaded ? "YES" : "NO");

    // 2. Unload first if already loaded to ensure we start clean
    if (initial_loaded) {
        printf("[TEST] Unloading existing module to start clean...\n");
        kernel_mgr_unload_module();
    }

    // Verify /proc/sysmgr is not accessible
    printf("[TEST] Verifying /proc/sysmgr access when not loaded...\n");
    int access_res = access("/proc/sysmgr", F_OK);
    if (access_res != 0) {
        printf("PASS: /proc/sysmgr is not available.\n");
    } else {
        printf("FAIL: /proc/sysmgr is still available when module is not loaded.\n");
    }

    // 3. Test Load Kernel Module (with sudo)
    printf("\n[TEST] Loading kernel module (expecting compilation if missing, and load)...\n");
    int load_res = kernel_mgr_load_module();
    if (load_res == 0) {
        printf("PASS: kernel_mgr_load_module returned success.\n");
    } else {
        printf("FAIL: kernel_mgr_load_module returned failure (%d).\n", load_res);
    }

    // 4. Verify /proc/sysmgr access when loaded
    printf("[TEST] Verifying /proc/sysmgr access when loaded...\n");
    access_res = access("/proc/sysmgr", F_OK);
    if (access_res == 0) {
        printf("PASS: /proc/sysmgr is available.\n");
    } else {
        printf("FAIL: /proc/sysmgr is not available when module is loaded.\n");
    }

    // 5. Test Repeated Load
    printf("\n[TEST] Loading kernel module again (repeated load)...\n");
    int rep_load_res = kernel_mgr_load_module();
    if (rep_load_res == 0) {
        printf("PASS: kernel_mgr_load_module handled repeated load successfully (returned 0).\n");
    } else {
        printf("FAIL: kernel_mgr_load_module failed on repeated load (%d).\n", rep_load_res);
    }

    // 6. Verify Status Detection
    printf("\n[TEST] Status detection check...\n");
    int status_res = kernel_mgr_show_status();
    if (status_res == 0) {
        printf("PASS: Status displayed successfully.\n");
    } else {
        printf("FAIL: Status display failed.\n");
    }

    // 7. Test Unload Module
    printf("\n[TEST] Unloading kernel module...\n");
    int unload_res = kernel_mgr_unload_module();
    if (unload_res == 0) {
        printf("PASS: kernel_mgr_unload_module returned success.\n");
    } else {
        printf("FAIL: kernel_mgr_unload_module returned failure (%d).\n", unload_res);
    }

    // Verify /proc/sysmgr no longer accessible
    printf("[TEST] Verifying /proc/sysmgr access after unload...\n");
    access_res = access("/proc/sysmgr", F_OK);
    if (access_res != 0) {
        printf("PASS: /proc/sysmgr is no longer available.\n");
    } else {
        printf("FAIL: /proc/sysmgr is still available after unload.\n");
    }

    // 8. Test Repeated Unload
    printf("\n[TEST] Unloading kernel module again (repeated unload)...\n");
    int rep_unload_res = kernel_mgr_unload_module();
    if (rep_unload_res == 0) {
        printf("PASS: kernel_mgr_unload_module handled repeated unload successfully (returned 0).\n");
    } else {
        printf("FAIL: kernel_mgr_unload_module failed on repeated unload (%d).\n", rep_unload_res);
    }

    // 9. Test Permission Failures (insmod without sudo)
    printf("\n[TEST] Testing permission failure (loading without sudo privileges)...\n");
    int perm_fail_res = kernel_mgr_load_module_no_sudo();
    if (perm_fail_res != 0) {
        printf("PASS: kernel_mgr_load_module_no_sudo failed as expected (returned %d).\n", perm_fail_res);
    } else {
        printf("FAIL: kernel_mgr_load_module_no_sudo unexpectedly succeeded (returned 0).\n");
    }

    printf("\nKernel Module Manager tests completed.\n");
    return 0;
}

