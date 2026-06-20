/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: cli/palette.h
 * Purpose: Searchable Command Palette descriptors and functions.
 */

#ifndef PALETTE_H
#define PALETTE_H

typedef struct {
    const char* command;     /* Executable command token (e.g. "search") */
    const char* display;     /* Human-readable menu option (e.g. "Search Package") */
    const char* context;     /* Context namespace ("main", "file", "process", etc.) */
} palette_item_t;

extern const palette_item_t palette_items[];
extern const int palette_items_count;

/**
 * Searches the command palette in the current context using the search string.
 * Prints matching commands in a clean visual layout.
 */
void palette_search_and_show(const char* current_context, const char* search_query);

#endif /* PALETTE_H */
