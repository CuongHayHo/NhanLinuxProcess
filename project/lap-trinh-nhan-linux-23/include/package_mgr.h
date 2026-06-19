/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: include/package_mgr.h
 * Purpose: Package installer wrapper.
 */

#ifndef PACKAGE_MGR_H
#define PACKAGE_MGR_H

#include "common.h"

void package_mgr_run(void);
int package_mgr_install(const char* pkg);
int package_mgr_remove(const char* pkg);
int package_mgr_search(const char* pkg);

#endif /* PACKAGE_MGR_H */
