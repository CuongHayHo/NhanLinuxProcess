/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/file_mgr.h
 * Purpose: File manager shell-based wrappers.
 */

#ifndef FILE_MGR_H
#define FILE_MGR_H

#include <sys/stat.h>
#include "common.h"

void file_mgr_run(void);
int file_mgr_create(const char* path);
int file_mgr_read(const char* path, char* buffer, size_t size);
int file_mgr_write(const char* path, const char* data, size_t size);
int file_mgr_delete(const char* path);
int file_mgr_rename(const char* old_path, const char* new_path);
int file_mgr_copy(const char* src_path, const char* dest_path);
int file_mgr_move(const char* src_path, const char* dest_path);
int file_mgr_get_info(const char* path, struct stat* statbuf);
int file_mgr_list_dir(const char* path);

int file_mgr_mkdir(const char* path);
int file_mgr_chmod(const char* path, const char* mode);
int file_mgr_search(const char* start_dir, const char* pattern);
int file_mgr_archive(const char* archive_path, const char* src_dir);

#endif /* FILE_MGR_H */

