#include "process/process.h"
#include "wacc/run.h"
#include "wacc/test/collect.h"
#include "wacc/test/test.h"

#include <assert.h>
#include <unistd.h>

static TEST_FUNC(state, execute, TestCase test)
{
    char* args[] = {"wacc", "-o", "wacc.out", (char*)test.path.ptr};
    int outpipe[2];
    int errpipe[2];
    assert(pipe(outpipe) == 0);
    assert(pipe(errpipe) == 0);
    FILE* out = fdopen(outpipe[1], "w");
    FILE* err = fdopen(errpipe[1], "w");
    int res = run((WaccArgBuf)BUF_ARRAY(args), out, err);
    (void)fclose(out);
    (void)fclose(err);
    if (test.valid)
    {
        if (res != 0)
        {
            if (test.skip_on_failure)
            {
                (void)remove("wacc.out");
                SKIP();
            }
            FILE* err_in = fdopen(errpipe[0], "r");
            char* line = NULL;
            size_t len = 0;
            while (getline(&line, &len, err_in) != -1)
            {
                (void)fprintf(stderr, "%s", line);
            }
            FAIL(state, CLEANUP((void)remove("wacc.out")), "wacc failed to compile valid test");
        }
    }
    else
    {
        TEST_ASSERT(state, res != 0, CLEANUP((void)remove("wacc.out")), "wacc compiled invalid test");
        PASS();
    }

    const char* run_args[] = {"./wacc.out"};
    ProcessCreateResult wacc_result =
        process_run((ProcessCStrBuf)BUF_ARRAY(run_args), PROCESS_OPTION_COMBINED_STDOUT_STDERR);
    TEST_ASSERT(state, wacc_result.present, CLEANUP((void)remove("wacc.out")), "wacc failed to run");

    int wacc_code = wacc_result.value.returnCode;
    process_destroy(&wacc_result.value);
    (void)remove("wacc.out");

    const char* gcc_args[] = {"gcc", "-o", "gcc.out", (char*)test.path.ptr};
    ProcessCreateResult gcc_result = process_run(
        (ProcessCStrBuf)BUF_ARRAY(gcc_args), PROCESS_OPTION_COMBINED_STDOUT_STDERR | PROCESS_OPTION_SEARCH_USER_PATH);
    TEST_ASSERT(state, gcc_result.present, CLEANUP((void)remove("gcc.out")), "gcc failed to run");
    process_destroy(&gcc_result.value);

    const char* gcc_run_args[] = {"./gcc.out"};
    ProcessCreateResult gcc_run_result =
        process_run((ProcessCStrBuf)BUF_ARRAY(gcc_run_args), PROCESS_OPTION_COMBINED_STDOUT_STDERR);
    TEST_ASSERT(state, gcc_run_result.present, CLEANUP((void)remove("gcc.out")), "gcc failed to run");

    int gcc_code = gcc_run_result.value.returnCode;
    process_destroy(&gcc_run_result.value);

    (void)remove("gcc.out");

    TEST_ASSERT(state,
        wacc_code == gcc_code,
        NO_CLEANUP,
        "wacc and gcc produced different results: %d (wacc) vs %d (gcc)",
        wacc_code,
        gcc_code);
    PASS();
}

static SUITE_FUNC(state, wacc)
{
    TestCaseBuf cases = collect_tests();
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
    RUN_SUITE(state, wacc, str_lit("wacc"));
}

int main()
{
    TestState state = {0};
    run_all(&state);
    printf("passed %zu, failed %zu, skipped %zu, assertions %zu\n",
        state.passed,
        state.failed,
        state.skipped,
        state.assertions);
    return state.failed > 0;
}
