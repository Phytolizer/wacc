#pragma once

#include "wacc/range.h"

#include <buf/buf.h>
#include <stdio.h>
#include <str/str.h>

typedef BUF(size_t) LineStartBuf;

typedef BUF(char) Text;

typedef struct
{
    str path;
    FILE* fp;
    Text text;
    LineStartBuf line_starts;
    size_t num_errors;
} Source;

typedef struct
{
    Source source;
} WaccSystem;

typedef enum
{
#define X(x) ERROR_##x,
#include "wacc/system/errors.def"
#undef X
} ErrorKind;

WaccSystem* wacc_system_new(void);
void wacc_system_free(WaccSystem* system);
void wacc_system_open_file(WaccSystem* system, str path);
int wacc_system_read_source(WaccSystem* system);
void wacc_system_handle_error(WaccSystem* system, ErrorKind error, Range range);
