#pragma once

#include "str/str.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef TEST_ABORT_ON_FAILURE
#define TEST_ABORT_ON_FAILURE 0
#endif

typedef struct
{
    uint64_t passed;
    uint64_t failed;
    uint64_t skipped;
    uint64_t assertions;
} TestState;

typedef enum
{
#define X(x) TEST_RESULT_##x,
#include "test_results.def"

#undef X
} TestResultType;

typedef struct
{
    TestResultType type;
    str errorMessage;
} TestResult;

#define TEST_FAIL(msg) ((TestResult){.type = TEST_RESULT_FAIL, .errorMessage = msg})
#define TEST_PASS() ((TestResult){.type = TEST_RESULT_PASS})
#define TEST_SKIP() ((TestResult){.type = TEST_RESULT_SKIP})

#if TEST_ABORT_ON_FAILURE
#define TEST_ABORT() abort()
#else
#define TEST_ABORT()
#endif
#define TEST_ASSERT(state, test, cleanup, ...) \
    do \
    { \
        ++(state)->assertions; \
        if (!(test)) \
        { \
            str message = str_printf(__VA_ARGS__); \
            cleanup; \
            TEST_ABORT(); \
            return TEST_FAIL(message); \
        } \
    } while (false)

#define FAIL(state, cleanup, ...) TEST_ASSERT(state, false, cleanup, __VA_ARGS__)

#define NO_CLEANUP (void)0
#define CLEANUP(x) x

#define RUN_SUITE(state, name, displayname) \
    do \
    { \
        (void)fprintf(stderr, "SUITE " str_fmt "\n", str_arg(displayname)); \
        str_free(displayname); \
        name##_test_suite(state); \
    } while (false)

#define RUN_TEST(state, name, displayname, ...) \
    do \
    { \
        str disp = displayname; \
        (void)fprintf(stderr, "TEST  " str_fmt "\n", str_arg(disp)); \
        TestResult result = name##_test(state, __VA_ARGS__); \
        switch (result.type) \
        { \
            case TEST_RESULT_FAIL: \
                ++(state)->failed; \
                (void)fprintf( \
                    stderr, "FAIL  " str_fmt ": " str_fmt "\n", str_arg(disp), str_arg(result.errorMessage)); \
                str_free(result.errorMessage); \
                break; \
            case TEST_RESULT_PASS: \
                ++(state)->passed; \
                break; \
            case TEST_RESULT_SKIP: \
                ++(state)->skipped; \
                (void)fprintf(stderr, "SKIP  " str_fmt "\n", str_arg(disp)); \
                break; \
        } \
        str_free(disp); \
    } while (false)

#define RUN_SUBTEST(state, name, cleanup, ...) \
    do \
    { \
        str message = name##_subtest(state, __VA_ARGS__); \
        if (str_len(message) > 0) \
        { \
            cleanup; \
            return message; \
        } \
    } while (false)

#define SUITE_FUNC(state, name) void name##_test_suite(TestState* state)

#define TEST_FUNC(state, name, ...) TestResult name##_test(TestState* state __VA_OPT__(, ) __VA_ARGS__)

#define SUBTEST_FUNC(state, name, ...) TestResult name##_subtest(TestState* state __VA_OPT__(, ) __VA_ARGS__)

#define PASS() return TEST_PASS()

#define SKIP() return TEST_SKIP()
