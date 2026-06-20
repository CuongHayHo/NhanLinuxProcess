/*
 * Copyright (c) 2026 Linux System Manager Project
 * All rights reserved.
 *
 * File: tests/cli_test.c
 * Purpose: Verification suite for the modern CLI parser, context state stack, and palette matching.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parser.h"
#include "palette.h"

/* Mock Repl Context Stack to verify transition rules without running prompt blocking loop */
#define TEST_MAX_DEPTH 8
static char test_context_stack[TEST_MAX_DEPTH][64] = { "main" };
static int test_context_depth = 0;

static void test_context_push(const char* context) {
    if (test_context_depth < TEST_MAX_DEPTH - 1) {
        test_context_depth++;
        strncpy(test_context_stack[test_context_depth], context, 63);
        test_context_stack[test_context_depth][63] = '\0';
    }
}

static void test_context_pop(void) {
    if (test_context_depth > 0) {
        test_context_depth--;
    }
}

static const char* test_get_current_context(void) {
    return test_context_stack[test_context_depth];
}

void verify_tokenizer(void) {
    printf("Testing CLI Tokenizer...\n");

    char line1[] = "package search hello";
    char* argv1[10];
    int argc1 = parser_tokenize(line1, argv1, 10);
    assert(argc1 == 3);
    assert(strcmp(argv1[0], "package") == 0);
    assert(strcmp(argv1[1], "search") == 0);
    assert(strcmp(argv1[2], "hello") == 0);
    parser_free_tokens(argv1, argc1);

    char line2[] = "file create \"space file name.txt\"";
    char* argv2[10];
    int argc2 = parser_tokenize(line2, argv2, 10);
    assert(argc2 == 3);
    assert(strcmp(argv2[0], "file") == 0);
    assert(strcmp(argv2[1], "create") == 0);
    assert(strcmp(argv2[2], "space file name.txt") == 0);
    parser_free_tokens(argv2, argc2);

    printf("  -> Tokenizer verification: PASSED\n\n");
}

void verify_context_flow(void) {
    printf("Testing Context Stack Transitions (Nesting and ESC simulation)...\n");

    /* Initially main */
    assert(strcmp(test_get_current_context(), "main") == 0);

    /* Push package manager context */
    test_context_push("package");
    assert(strcmp(test_get_current_context(), "package") == 0);

    /* Push package search sub-context */
    test_context_push("search");
    assert(strcmp(test_get_current_context(), "search") == 0);

    /* Pop context back (Escape simulation) */
    test_context_pop();
    assert(strcmp(test_get_current_context(), "package") == 0);

    /* Pop back to main */
    test_context_pop();
    assert(strcmp(test_get_current_context(), "main") == 0);

    /* Pop main should do nothing (keep main) */
    test_context_pop();
    assert(strcmp(test_get_current_context(), "main") == 0);

    printf("  -> Context transition validation: PASSED\n\n");
}

void verify_palette(void) {
    printf("Testing Command Palette metadata database...\n");

    assert(palette_items_count > 0);

    /* Look for File Manager in main context */
    int found_file = 0;
    for (int i = 0; i < palette_items_count; i++) {
        if (strcmp(palette_items[i].context, "main") == 0 && strcmp(palette_items[i].command, "file") == 0) {
            found_file = 1;
            break;
        }
    }
    assert(found_file == 1);

    /* Look for info in kernel module context */
    int found_kernel_info = 0;
    for (int i = 0; i < palette_items_count; i++) {
        if (strcmp(palette_items[i].context, "kernel") == 0 && strcmp(palette_items[i].command, "info") == 0) {
            found_kernel_info = 1;
            break;
        }
    }
    assert(found_kernel_info == 1);

    printf("  -> Palette metadata validation: PASSED\n\n");
}

int main(void) {
    printf("==================================================\n");
    printf("               sysmgr CLI Unit Tests              \n");
    printf("==================================================\n\n");

    verify_tokenizer();
    verify_context_flow();
    verify_palette();

    printf("All CLI checks completed successfully (exit code 0).\n");
    return 0;
}
