#ifndef MINUNIT_H
#define MINUNIT_H

#include <stdio.h>
#include <stdlib.h>

/* Minimal single-file unit test helper (micro-minunit style) */
static int mu_tests_run = 0;
static int mu_tests_failed = 0;

#define mu_assert(msg, expr) do { \
    if (!(expr)) { \
        printf("FAIL: %s:%d: %s\n", __FILE__, __LINE__, (msg)); \
        mu_tests_failed++; \
        return 1; \
    } \
} while (0)

#define mu_assert_strcmp(msg, expected, actual) do { \
    if (strcmp((expected), (actual)) != 0) { \
        printf("FAIL: %s:%d: Expected '%s', got '%s': %s\n", \
               __FILE__, __LINE__, (expected), (actual), (msg)); \
        mu_tests_failed++; \
        return 1; \
    } \
} while (0)

#define mu_run_test(test) do { \
    int rc = test(); \
    mu_tests_run++; \
    if (rc) return rc; \
} while (0)

#define mu_summary() do { \
    if (mu_tests_failed == 0) { \
        printf("=========================================================PASS: %d tests\n\n\n", mu_tests_run); \
        exit(0); \
    } else { \
        printf("=======================================================FAILED: %d failed, %d run\n\n\n", mu_tests_failed, mu_tests_run); \
        exit(1); \
    } \
} while (0)

#endif /* MINUNIT_H */