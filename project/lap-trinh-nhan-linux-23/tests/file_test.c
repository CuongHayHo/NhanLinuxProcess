/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/file_test.c
 * Purpose: Complete Diagnostic tests and demonstrations for File Manager (Phases A, B, & C).
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "file_mgr.h"

int main(void) {
    const char* dir_path = "tests/test_dir";
    const char* file1 = "tests/test_dir/file1.txt";
    const char* file2 = "tests/test_dir/file2.txt";
    const char* file1_copy = "tests/test_dir/file1_copy.txt";
    const char* file2_moved = "tests/test_dir/file2_moved.txt";
    
    char read_buf[256];
    int result;

    printf("Starting File Manager - Phase C test program...\n");

    /* 1. Create Directory */
    printf("\n1. Creating directory: %s...\n", dir_path);
    /* Remove it first if it exists from a crashed run */
    rmdir(dir_path);
    result = mkdir(dir_path, 0755);
    if (result != 0) {
        perror("mkdir failed");
        return 1;
    }
    printf("Directory created successfully.\n");

    /* 2. Create and Write Files */
    printf("\n2. Creating and writing files inside %s...\n", dir_path);
    file_mgr_write(file1, "Content of File 1", 17);
    file_mgr_write(file2, "Content of File 2", 17);

    /* 3. List Directory */
    printf("\n3. Listing directory contents:\n");
    result = file_mgr_list_dir(dir_path);
    if (result != 0) {
        printf("Failed to list directory!\n");
        return 1;
    }

    /* 4. Copy File */
    printf("\n4. Copying %s to %s...\n", file1, file1_copy);
    result = file_mgr_copy(file1, file1_copy);
    if (result != 0) {
        printf("Failed to copy file!\n");
        return 1;
    }
    
    /* Verify copy contents */
    memset(read_buf, 0, sizeof(read_buf));
    file_mgr_read(file1_copy, read_buf, sizeof(read_buf));
    printf("Verify copied file content: '%s'\n", read_buf);
    if (strcmp(read_buf, "Content of File 1") != 0) {
        printf("Copied content mismatch!\n");
        return 1;
    }

    /* 5. Move File */
    printf("\n5. Moving %s to %s...\n", file2, file2_moved);
    result = file_mgr_move(file2, file2_moved);
    if (result != 0) {
        printf("Failed to move file!\n");
        return 1;
    }
    
    /* Verify moved contents and source deletion */
    memset(read_buf, 0, sizeof(read_buf));
    file_mgr_read(file2_moved, read_buf, sizeof(read_buf));
    printf("Verify moved file content: '%s'\n", read_buf);
    if (strcmp(read_buf, "Content of File 2") != 0) {
        printf("Moved content mismatch!\n");
        return 1;
    }
    if (access(file2, F_OK) != 0) {
        printf("Source file '%s' deleted successfully after move.\n", file2);
    } else {
        printf("Error: Source file '%s' still exists after move!\n", file2);
        return 1;
    }

    /* 6. List Directory post-changes */
    printf("\n6. Listing directory after copy & move operations:\n");
    file_mgr_list_dir(dir_path);

    /* Clean up all created files and directory */
    printf("\nCleaning up test directory and files...\n");
    file_mgr_delete(file1);
    file_mgr_delete(file1_copy);
    file_mgr_delete(file2_moved);
    rmdir(dir_path);

    printf("File Manager tests completed successfully.\n");
    return 0;
}
