/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: cli/autocomplete.h
 * Purpose: Tab autocompleter callbacks.
 */

#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

/**
 * Initializes and registers the autocompletion callbacks with linenoise.
 */
void autocomplete_init(void);

#endif /* AUTOCOMPLETE_H */
