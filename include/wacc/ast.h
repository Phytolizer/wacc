#pragma once

#include <buf/buf.h>
#include <stdint.h>
#include <str/str.h>

typedef enum
{
#define X(x) WACC_EXPR_KIND_##x,
#include "wacc/ast/expr_kinds.def"
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
#include "wacc/ast/unary_op_kinds.def"
#undef X
} WaccUnaryOperation;

typedef struct
{
    WaccExpression base;
    WaccUnaryOperation op;
    WaccExpression* expr;
} WaccUnaryExpression;

typedef enum
{
#define X(x) WACC_BINARY_OP_##x,
#include "wacc/ast/binary_op_kinds.def"
#undef X
} WaccBinaryOperation;

typedef struct
{
    WaccExpression base;
    WaccBinaryOperation op;
    WaccExpression* lhs;
    WaccExpression* rhs;
} WaccBinaryExpression;

typedef struct
{
    WaccExpression base;
    str name;
} WaccVariableExpression;

typedef struct
{
    WaccExpression base;
    str name;
    WaccExpression* expr;
} WaccAssignmentExpression;

typedef enum
{
#define X(x) WACC_STMT_KIND_##x,
#include "wacc/ast/stmt_kinds.def"
#undef X
} WaccStatementKind;

typedef struct
{
    WaccStatementKind kind;
} WaccStatement;

typedef struct
{
    WaccStatement base;
    str name;
    WaccExpression* initializer;
} WaccDeclareStatement;

typedef struct
{
    WaccStatement base;
    WaccExpression* expression;
} WaccReturnStatement;

typedef struct
{
    WaccStatement base;
    WaccExpression* expression;
} WaccExpressionStatement;

typedef BUF(WaccStatement*) WaccStatementList;

typedef struct
{
    str name;
    WaccStatementList statements;
} WaccFunction;

typedef struct
{
    WaccFunction function;
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
        WaccFunction function;
        WaccStatement* statement;
        WaccStatementList statement_list;
        WaccExpression* expression;
        WaccUnaryOperation unary_op;
        WaccBinaryOperation binary_op;
    } as;
} WaccNode;
#define D_ParseNode_User WaccNode

WaccExpression* wacc_expr_new_unary(WaccUnaryOperation op, WaccExpression* expr);
WaccExpression* wacc_expr_new_constant(uint64_t value);
WaccExpression* wacc_expr_new_binary(WaccExpression* lhs, WaccBinaryOperation op, WaccExpression* rhs);
WaccExpression* wacc_expr_new_variable(str name);
WaccExpression* wacc_expr_new_assignment(str name, WaccExpression* expr);

WaccStatement* wacc_stmt_new_declare(str name, WaccExpression* initializer);
WaccStatement* wacc_stmt_new_return(WaccExpression* expression);
WaccStatement* wacc_stmt_new_expression(WaccExpression* expression);

void ast_show(WaccNode root, FILE* out, FILE* err);
void ast_free(WaccNode root);
