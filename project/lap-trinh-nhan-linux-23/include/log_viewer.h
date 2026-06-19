/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/log_viewer.h
 * Purpose: Diagnostic log file viewer and parser.
 */

#ifndef LOG_VIEWER_H
#define LOG_VIEWER_H

#include "common.h"

void log_viewer_run(void);
int log_viewer_display(const char* file_path);
int log_viewer_search(const char* file_path, const char* keyword);
int log_viewer_clear(const char* file_path);

#endif /* LOG_VIEWER_H */
