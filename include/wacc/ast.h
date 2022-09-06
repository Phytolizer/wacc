#pragma once

#include "str/str.h"

#include <stdint.h>

typedef struct
{
    uint64_t value;
} WaccExpression;

typedef struct
{
    WaccExpression expression;
} WaccStatement;

typedef struct
{
    str name;
    WaccStatement statement;
} WaccFunction;

typedef struct
{
    WaccFunction function;
} WaccProgram;

typedef enum
{
#define X(x) WACC_NODE_##x,
#include "wacc/node_kinds.def"
#undef X
} WaccNodeKind;

typedef struct
{
    WaccNodeKind kind;
    union {
        str text;
        WaccProgram program;
        WaccFunction function;
        WaccStatement statement;
        WaccExpression expression;
    } as;
} WaccNode;
