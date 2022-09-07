#pragma once

#include "str/str.h"

#include <stdint.h>

typedef enum
{
#define X(x) WACC_EXPR_KIND_##x,
#include "wacc/ast_expr_kinds.def"
#undef X
} WaccExpressionKind;

typedef struct
{
    WaccExpressionKind kind;
} WaccExpression;

typedef struct
{
    WaccExpression base;
    uint64_t value;
} WaccConstantExpression;

typedef enum
{
#define X(x) WACC_UNARY_OP_##x,
#include "wacc/ast_unary_op_kinds.def"
#undef X
} WaccUnaryOperation;

typedef struct
{
    WaccExpression base;
    WaccUnaryOperation op;
    WaccExpression* expr;
} WaccUnaryExpression;

typedef struct
{
    WaccExpression* expression;
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
        WaccExpression* expression;
        WaccUnaryOperation unary_op;
    } as;
} WaccNode;
#define D_ParseNode_User WaccNode

WaccExpression* wacc_expr_new_unary(WaccUnaryOperation op, WaccExpression* expr);
WaccExpression* wacc_expr_new_constant(uint64_t value);

void ast_show(WaccNode root, FILE* out, FILE* err);
void ast_free(WaccNode root);
