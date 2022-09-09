#pragma once

#include <buf/buf.h>
#include <stdbool.h>
#include <str/str.h>

typedef struct
{
    str path;
    bool valid;
    bool skip_on_failure;
} TestCase;

typedef BUF(TestCase) TestCaseBuf;

TestCaseBuf collect_tests(void);
