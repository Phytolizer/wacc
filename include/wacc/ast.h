#pragma once

#include "str/str.h"

#include <stdint.h>

typedef enum
{
#define X(x) WACC_EXPR_##x,
#include "wacc/ast/expression_types.def"
#undef X
} WaccExpressionType;

typedef struct
{
    WaccExpressionType type;
} WaccExpression;

typedef struct
{
    WaccExpression base;
    uint64_t value;
} WaccConstantExpression;

typedef struct
{
    WaccExpression* expression;
} WaccStatement;

typedef enum
{
#define X(x) WACC_FUNC_##x,
#include "wacc/ast/function_types.def"
#undef X
} WaccFunctionType;

typedef struct
{
    WaccFunctionType type;
} WaccFunction;

typedef struct
{
    WaccFunction base;
    str name;
    WaccStatement statement;
} WaccActualFunction;

typedef struct
{
    WaccFunction* function;
} WaccProgram;

typedef enum
{
#define X(x) WACC_NODE_##x,
#include "wacc/ast/node_kinds.def"
#undef X
} WaccNodeKind;

typedef struct
{
    WaccNodeKind kind;
    union {
        str text;
        WaccProgram program;
        WaccFunction* function;
        WaccStatement statement;
        WaccExpression* expression;
    } as;
} WaccNode;

WaccNode* wacc_node_new_text(str text);
WaccNode* wacc_node_new_program(WaccFunction* function);
WaccNode* wacc_node_new_function(str name, WaccStatement statement);
WaccNode* wacc_node_new_statement(WaccExpression* expression);
WaccNode* wacc_node_new_expression(uint64_t value);

WaccNode* wacc_error_node_function(void);
WaccNode* wacc_error_node_expression(void);

void ast_free(WaccNode* ast);
