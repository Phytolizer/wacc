#include "test.h"

#include <inttypes.h>
#include <stdio.h>
#include <str/str.h>

static void run_all(TestState* state)
{
    (void)state;
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
