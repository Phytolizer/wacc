#pragma once

#include <stdint.h>
typedef struct
{
    uint64_t start;
    uint64_t end;
} Range;

#define range_null ((Range){0, 0})
#define range_new(start, end) ((Range){(start), (end)})
