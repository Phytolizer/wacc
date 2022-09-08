#include "test.h"

#include "process/process.h"
#include "str/strtox.h"
#include "wacc/run.h"

#include <buf/buf.h>
#include <dirent.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <str/str.h>
#include <unistd.h>
#include <wacc-config.h>

enum
{
    IMPLEMENTED_STAGES = 5
};

typedef struct
{
    str path;
    bool valid;
    bool skip_on_failure;
} TestCase;

typedef struct
{
    bool is_valid;
    bool is_invalid;
} TestDirInfo;

typedef BUF(TestCase) TestCases;

#define str_after(s, ofs) str_ref_chars((s).ptr + (ofs), (s).len - (ofs))

static void walk_stage_dir(TestCases* buf, TestDirInfo* info, str path)
{
    DIR* dir = opendir(path.ptr);
    if (dir == NULL)
    {
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        str name = str_ref(entry->d_name);
        if (str_eq(name, str_lit(".")) || str_eq(name, str_lit("..")))
        {
            continue;
        }

        str child_path = str_null;
        path_join(&child_path, path, name);
        if (entry->d_type == DT_DIR)
        {
            if (str_eq(name, str_lit("invalid")))
            {
                info->is_invalid = true;
            }
            else if (str_eq(name, str_lit("valid")))
            {
                info->is_valid = true;
            }
            walk_stage_dir(buf, info, str_ref(child_path));
            str_free(child_path);
        }
        else if (entry->d_type == DT_REG)
        {
            if (str_has_suffix(name, str_lit(".c")))
            {
                TestCase testCase;
                testCase.path = child_path;
                testCase.valid = info->is_valid;
                testCase.skip_on_failure = str_has_prefix(name, str_lit("skip_on_failure_"));
                BUF_PUSH(buf, testCase);
            }
        }
    }

    closedir(dir);
}

static TestCases get_tests(void)
{
    TestCases result = BUF_NEW;
    str top = str_lit(PROJECT_SOURCE_DIR "/tests/cases");

    DIR* top_dir = opendir(top.ptr);
    if (top_dir == NULL)
    {
        (void)fprintf(stderr, "Failed to open test cases directory: " str_fmt "\n", str_arg(top));
        exit(1);
    }

    str prefix = str_lit("stage");

    struct dirent* entry;
    while ((entry = readdir(top_dir)) != NULL)
    {
        if (entry->d_type != DT_DIR)
        {
            continue;
        }
        str name = str_ref(entry->d_name);
        if (!str_has_prefix(name, prefix))
        {
            continue;
        }
        str namep = str_after(name, str_len(prefix));
        Str2U64Result stage = str2u64(namep, 10);
        if (stage.err != 0)
        {
            continue;
        }
        if (stage.value <= IMPLEMENTED_STAGES)
        {
            str stage_path = str_null;
            path_join(&stage_path, top, name);
            TestDirInfo info = {0};
            walk_stage_dir(&result, &info, str_ref(stage_path));
            str_free(stage_path);
        }
    }

    closedir(top_dir);

    return result;
}

static TEST_FUNC(state, execute, TestCase info)
{
    char* args[] = {"wacc", "-o", "wacc.out", (char*)info.path.ptr};
    int outpipe[2];
    int errpipe[2];
    TEST_ASSERT(state, pipe(outpipe) == 0, NO_CLEANUP, "failed to create pipe: %s", strerror(errno));
    TEST_ASSERT(state, pipe(errpipe) == 0, NO_CLEANUP, "failed to create pipe: %s", strerror(errno));
    FILE* out = fdopen(outpipe[1], "w");
    FILE* err = fdopen(errpipe[1], "w");
    int res = run((Argv)BUF_ARRAY(args), out, err);
    (void)fclose(out);
    (void)fclose(err);
    if (info.valid)
    {
        if (res != 0)
        {
            if (info.skip_on_failure)
            {
                (void)remove("wacc.out");
                SKIP();
            }
            FILE* err_in = fdopen(errpipe[0], "r");
            char* line = NULL;
            size_t len = 0;
            ssize_t nread;
            while ((nread = getline(&line, &len, err_in)) != -1)
            {
                (void)fprintf(stderr, "%s", line);
            }
            FAIL(state, CLEANUP((void)remove("wacc.out")), "wacc failed to compile");
        }
    }
    else
    {
        TEST_ASSERT(state,
            res != 0,
            CLEANUP((void)remove("wacc.out")),
            "wacc compiled invalid test " str_fmt,
            str_arg(info.path));
        PASS();
    }

    const char* run_args[] = {"./wacc.out"};
    ProcessCreateResult wacc_result =
        process_run((ProcessCStrBuf)BUF_ARRAY(run_args), PROCESS_OPTION_COMBINED_STDOUT_STDERR);
    TEST_ASSERT(state, wacc_result.present, CLEANUP((void)remove("wacc.out")), "wacc program failed to spawn");

    int wacc_code = wacc_result.value.returnCode;
    process_destroy(&wacc_result.value);
    (void)remove("wacc.out");

    const char* gcc_args[] = {"gcc", info.path.ptr, "-o", "gcc.out"};
    ProcessCreateResult gcc_result = process_run(
        (ProcessCStrBuf)BUF_ARRAY(gcc_args), PROCESS_OPTION_COMBINED_STDOUT_STDERR | PROCESS_OPTION_SEARCH_USER_PATH);
    TEST_ASSERT(
        state, gcc_result.present, CLEANUP((void)remove("wacc.out"); (void)remove("gcc.out")), "gcc failed to compile");
    process_destroy(&gcc_result.value);

    const char* gcc_run_args[] = {"./gcc.out"};
    ProcessCreateResult gcc_run_result =
        process_run((ProcessCStrBuf)BUF_ARRAY(gcc_run_args), PROCESS_OPTION_COMBINED_STDOUT_STDERR);
    TEST_ASSERT(state, gcc_run_result.present, CLEANUP((void)remove("gcc.out")), "gcc program failed to spawn");

    int gcc_code = gcc_run_result.value.returnCode;
    process_destroy(&gcc_run_result.value);

    (void)remove("gcc.out");

    TEST_ASSERT(state,
        wacc_code == gcc_code,
        NO_CLEANUP,
        "wacc and gcc produced different exit codes: %d vs %d",
        wacc_code,
        gcc_code);

    PASS();
}

static SUITE_FUNC(state, compare_with_gcc)
{
    TestCases cases = get_tests();
    for (uint64_t i = 0; i < cases.len; i++)
    {
        TestCase test = cases.ptr[i];
        RUN_TEST(state, execute, str_printf("test case " str_fmt, str_arg(test.path)), test);
        str_free(test.path);
    }
    BUF_FREE(cases);
}

static void run_all(TestState* state)
{
    RUN_SUITE(state, compare_with_gcc, str_lit("Compare output with GCC"));
}

int main(void)
{
    TestState state = {0};
    run_all(&state);
    printf("passed: %" PRIu64 ", failed: %" PRIu64 ", skipped: %" PRIu64 ", assertions: %" PRIu64 "\n",
        state.passed,
        state.failed,
        state.skipped,
        state.assertions);
    return state.failed > 0;
}
