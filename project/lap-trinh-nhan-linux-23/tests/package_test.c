/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/package_test.c
 * Purpose: Diagnostic unit tests for Package Manager.
 */

#include <stdio.h>
#include "package_mgr.h"

int main(void) {
    printf("Starting Package Manager Module - Sprint 3 (Package Information) test program...\n\n");
    
    printf("--- Test 1: Package Manager Detection ---\n");
    const char* pm = package_mgr_detect();
    if (pm) {
        printf("Detected Package Manager: %s\n", pm);
    } else {
        printf("Package manager not supported.\n");
    }
    
    printf("\n--- Test 2: List Installed Packages ---\n");
    int list_res = package_mgr_list_installed();
    
    printf("\n--- Test 3: Search Existing Package ('kernel') ---\n");
    int search_exist_res = package_mgr_search("kernel");
    
    printf("\n--- Test 4: Search Missing Package ('nonexistent_pkg_xyz') ---\n");
    int search_missing_res = package_mgr_search("nonexistent_pkg_xyz");
    
    printf("\n--- Test 5: Case-Insensitive Search ('KERNEL') ---\n");
    int search_case_res = package_mgr_search("KERNEL");
    
    printf("\n--- Test 6: Partial Name Search ('kern') ---\n");
    int search_partial_res = package_mgr_search("kern");

    printf("\n--- Test 7: Package Info on Existing Package ('rpm') ---\n");
    int info_exist_res = package_mgr_info("rpm");

    printf("\n--- Test 8: Package Info on Missing Package ('nonexistent_pkg_xyz') ---\n");
    int info_missing_res = package_mgr_info("nonexistent_pkg_xyz");

    printf("\n--- Test 9: Package Info on Unsafe Package Name ('; rm -rf /') ---\n");
    int info_unsafe_res = package_mgr_info("; rm -rf /");
    
    // Check results
    if (list_res == 0 && search_exist_res == 0 && search_missing_res == 0 && 
        search_case_res == 0 && search_partial_res == 0 && info_exist_res == 0 &&
        info_missing_res == -1 && info_unsafe_res == -1) {
        printf("\nPackage Manager Module tests completed successfully.\n");
        return 0;
    } else {
        printf("\nPackage Manager Module tests failed.\n");
        printf("Results: list_res=%d, search_exist_res=%d, search_missing_res=%d, search_case_res=%d, search_partial_res=%d, info_exist_res=%d, info_missing_res=%d, info_unsafe_res=%d\n",
               list_res, search_exist_res, search_missing_res, search_case_res, search_partial_res, info_exist_res, info_missing_res, info_unsafe_res);
        return 1;
    }
}
