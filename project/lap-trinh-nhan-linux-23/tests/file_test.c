/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/file_test.c
 * Purpose: Complete Diagnostic regression tests for File Manager via shell wrappers.
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
    const char* file1_copy = "tests/test_dir/file1_copy.txt";
    const char* file1_moved = "tests/test_dir/file1_moved.txt";
    const char* file1_renamed = "tests/test_dir/file1_renamed.txt";
    const char* archive_file = "tests/test_dir/archive.tar.gz";
    
    char read_buf[256];
    int result;

    printf("Starting File Manager Shell Programming Regression Tests...\n\n");

    // 1. Verify script exists
    printf("[TEST 1] Checking if shell/file.sh exists...\n");
    if (access("shell/file.sh", F_OK) == 0) {
        printf("PASS: shell/file.sh exists.\n");
    } else {
        printf("FAIL: shell/file.sh is missing!\n");
        return 1;
    }

    // 2. Verify launcher works (mkdir)
    printf("[TEST 2] Verifying launcher and mkdir...\n");
    // Ensure clean state using shell delete wrapper first
    file_mgr_delete(dir_path);
    result = file_mgr_mkdir(dir_path);
    if (result == 0) {
        printf("PASS: file_mgr_mkdir succeeded.\n");
    } else {
        printf("FAIL: file_mgr_mkdir failed.\n");
        return 1;
    }

    // 3. Verify create
    printf("[TEST 3] Verifying file creation...\n");
    result = file_mgr_create(file1);
    if (result == 0) {
        printf("PASS: file_mgr_create succeeded.\n");
    } else {
        printf("FAIL: file_mgr_create failed.\n");
        return 1;
    }

    // 4. Verify write
    printf("[TEST 4] Verifying file write...\n");
    result = file_mgr_write(file1, "Content of File 1", 17);
    if (result == 17) {
        printf("PASS: file_mgr_write succeeded.\n");
    } else {
        printf("FAIL: file_mgr_write failed.\n");
        return 1;
    }

    // 5. Verify read
    printf("[TEST 5] Verifying file read...\n");
    memset(read_buf, 0, sizeof(read_buf));
    result = file_mgr_read(file1, read_buf, sizeof(read_buf));
    if (result >= 0 && strcmp(read_buf, "Content of File 1") == 0) {
        printf("PASS: file_mgr_read succeeded (content: '%s').\n", read_buf);
    } else {
        printf("FAIL: file_mgr_read content mismatch (content: '%s').\n", read_buf);
        return 1;
    }

    // 6. Verify copy
    printf("[TEST 6] Verifying file copy...\n");
    result = file_mgr_copy(file1, file1_copy);
    if (result == 0) {
        memset(read_buf, 0, sizeof(read_buf));
        file_mgr_read(file1_copy, read_buf, sizeof(read_buf));
        if (strcmp(read_buf, "Content of File 1") == 0) {
            printf("PASS: file_mgr_copy succeeded.\n");
        } else {
            printf("FAIL: file_mgr_copy verified content mismatch.\n");
            return 1;
        }
    } else {
        printf("FAIL: file_mgr_copy wrapper returned error.\n");
        return 1;
    }

    // 7. Verify move
    printf("[TEST 7] Verifying file move...\n");
    result = file_mgr_move(file1_copy, file1_moved);
    if (result == 0) {
        memset(read_buf, 0, sizeof(read_buf));
        file_mgr_read(file1_moved, read_buf, sizeof(read_buf));
        if (strcmp(read_buf, "Content of File 1") == 0) {
            printf("PASS: file_mgr_move succeeded.\n");
        } else {
            printf("FAIL: file_mgr_move verified content mismatch.\n");
            return 1;
        }
    } else {
        printf("FAIL: file_mgr_move wrapper returned error.\n");
        return 1;
    }

    // 8. Verify rename
    printf("[TEST 8] Verifying file rename...\n");
    result = file_mgr_rename(file1_moved, file1_renamed);
    if (result == 0) {
        memset(read_buf, 0, sizeof(read_buf));
        file_mgr_read(file1_renamed, read_buf, sizeof(read_buf));
        if (strcmp(read_buf, "Content of File 1") == 0) {
            printf("PASS: file_mgr_rename succeeded.\n");
        } else {
            printf("FAIL: file_mgr_rename verified content mismatch.\n");
            return 1;
        }
    } else {
        printf("FAIL: file_mgr_rename wrapper returned error.\n");
        return 1;
    }

    // 9. Verify stat (info)
    printf("[TEST 9] Verifying file info (stat)...\n");
    struct stat st;
    result = file_mgr_get_info(file1_renamed, &st);
    if (result == 0) {
        printf("PASS: file_mgr_get_info succeeded.\n");
    } else {
        printf("FAIL: file_mgr_get_info failed.\n");
        return 1;
    }

    // 10. Verify ls (list_dir)
    printf("[TEST 10] Verifying directory listing (ls)...\n");
    result = file_mgr_list_dir(dir_path);
    if (result == 0) {
        printf("PASS: file_mgr_list_dir succeeded.\n");
    } else {
        printf("FAIL: file_mgr_list_dir failed.\n");
        return 1;
    }

    // 11. Verify chmod
    printf("[TEST 11] Verifying permission modification (chmod)...\n");
    result = file_mgr_chmod(file1_renamed, "644");
    if (result == 0) {
        printf("PASS: file_mgr_chmod succeeded.\n");
    } else {
        printf("FAIL: file_mgr_chmod failed.\n");
        return 1;
    }

    // 12. Verify find (search)
    printf("[TEST 12] Verifying search files (find)...\n");
    result = file_mgr_search(dir_path, "file1_renamed.txt");
    if (result == 0) {
        printf("PASS: file_mgr_search succeeded.\n");
    } else {
        printf("FAIL: file_mgr_search failed.\n");
        return 1;
    }

    // 13. Verify tar (archive)
    printf("[TEST 13] Verifying file archiver (tar)...\n");
    result = file_mgr_archive(archive_file, file1_renamed);
    if (result == 0) {
        printf("PASS: file_mgr_archive succeeded.\n");
    } else {
        printf("FAIL: file_mgr_archive failed.\n");
        return 1;
    }

    // 14. Verify delete
    printf("[TEST 14] Verifying file cleanup and deletion...\n");
    int del_res1 = file_mgr_delete(file1);
    int del_res2 = file_mgr_delete(file1_renamed);
    int del_res3 = file_mgr_delete(archive_file);
    int del_res4 = file_mgr_delete(dir_path);
    if (del_res1 == 0 && del_res2 == 0 && del_res3 == 0 && del_res4 == 0) {
        printf("PASS: All file deletion cleanups succeeded.\n");
    } else {
        printf("FAIL: File deletion cleanups failed: %d, %d, %d, %d\n", del_res1, del_res2, del_res3, del_res4);
        return 1;
    }

    printf("\nAll File Manager regression tests completed successfully.\n");
    return 0;
}
