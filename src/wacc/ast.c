#include "wacc/ast.h"

#include <hedley.h>
#include <inttypes.h>
#include <stdlib.h>

static const char* UNARY_OP_STRINGS[] = {
#define X(x) #x,
#include "wacc/ast/unary_op_kinds.def"
#undef X
};

static const char* BINARY_OP_STRINGS[] = {
#define X(x) #x,
#include "wacc/ast/binary_op_kinds.def"
#undef X
};

typedef struct
{
    uint64_t depth;
} AstDumper;

static void show_indent(AstDumper* dumper, FILE* out)
{
    for (uint64_t i = 0; i < dumper->depth; i++)
    {
        (void)fprintf(out, "  ");
    }
}

static void show_expression(WaccExpression* expression, FILE* out)
{
    switch (expression->kind)
    {
        case WACC_EXPR_KIND_CONSTANT: {
            WaccConstantExpression* constant = (WaccConstantExpression*)expression;
            (void)fprintf(out, "%" PRIu64, constant->value);
            break;
        }
        case WACC_EXPR_KIND_UNARY: {
            WaccUnaryExpression* unary = (WaccUnaryExpression*)expression;
            (void)fprintf(out, "%s ", UNARY_OP_STRINGS[unary->op]);
            show_expression(unary->expr, out);
            break;
        }
        case WACC_EXPR_KIND_BINARY: {
            WaccBinaryExpression* binary = (WaccBinaryExpression*)expression;
            (void)fprintf(out, "(");
            show_expression(binary->lhs, out);
            (void)fprintf(out, " %s ", BINARY_OP_STRINGS[binary->op]);
            show_expression(binary->rhs, out);
            (void)fprintf(out, ")");
            break;
        }
        default: {
            (void)fprintf(out, "unknown expression");
            break;
        }
    }
}

static void show_return_statement(AstDumper* d, WaccReturnStatement* statement, FILE* out)
{
    show_indent(d, out);
    (void)fprintf(out, "RETURN:\n");
    d->depth++;
    show_indent(d, out);
    show_expression(statement->expression, out);
    (void)fprintf(out, "\n");
    d->depth--;
}

static void show_expression_statement(AstDumper* d, WaccExpressionStatement* statement, FILE* out)
{
    show_indent(d, out);
    (void)fprintf(out, "EXPRESSION:\n");
    d->depth++;
    show_indent(d, out);
    show_expression(statement->expression, out);
    (void)fprintf(out, "\n");
    d->depth--;
}

static void show_declare_statement(AstDumper* d, WaccDeclareStatement* statement, FILE* out)
{
    show_indent(d, out);
    (void)fprintf(out, "DECLARE: " str_fmt "\n", str_arg(statement->name));
    if (statement->initializer)
    {
        d->depth++;
        show_indent(d, out);
        show_expression(statement->initializer, out);
        (void)fprintf(out, "\n");
        d->depth--;
    }
}

static void show_statement(AstDumper* d, WaccStatement* statement, FILE* out)
{
    switch (statement->kind)
    {
        case WACC_STMT_KIND_RETURN:
            show_return_statement(d, (WaccReturnStatement*)statement, out);
            break;
        case WACC_STMT_KIND_EXPRESSION:
            show_expression_statement(d, (WaccExpressionStatement*)statement, out);
            break;
        case WACC_STMT_KIND_DECLARE:
            show_declare_statement(d, (WaccDeclareStatement*)statement, out);
            break;
        default: {
            (void)fprintf(out, "unknown statement");
            break;
        }
    }
}

static void show_function(AstDumper* d, WaccFunction function, FILE* out)
{
    (void)fprintf(out, "FUNCTION " str_fmt ":\n", str_arg(function.name));
    d->depth++;
    for (uint64_t i = 0; i < function.statements.len; i++)
    {
        show_statement(d, function.statements.ptr[i], out);
    }
    d->depth--;
}

static void show_program(AstDumper* d, WaccProgram program, FILE* out)
{
    show_function(d, program.function, out);
}

WaccExpression* wacc_expr_new_unary(WaccUnaryOperation op, WaccExpression* expr)
{
    WaccUnaryExpression* unary = malloc(sizeof(WaccUnaryExpression));
    unary->base.kind = WACC_EXPR_KIND_UNARY;
    unary->op = op;
    unary->expr = expr;
    return (WaccExpression*)unary;
}

WaccExpression* wacc_expr_new_constant(uint64_t value)
{
    WaccConstantExpression* constant = malloc(sizeof(WaccConstantExpression));
    constant->base.kind = WACC_EXPR_KIND_CONSTANT;
    constant->value = value;
    return (WaccExpression*)constant;
}

WaccExpression* wacc_expr_new_binary(WaccExpression* lhs, WaccBinaryOperation op, WaccExpression* rhs)
{
    WaccBinaryExpression* binary = malloc(sizeof(WaccBinaryExpression));
    binary->base.kind = WACC_EXPR_KIND_BINARY;
    binary->op = op;
    binary->lhs = lhs;
    binary->rhs = rhs;
    return (WaccExpression*)binary;
}

WaccExpression* wacc_expr_new_variable(str name)
{
    WaccVariableExpression* variable = malloc(sizeof(WaccVariableExpression));
    variable->base.kind = WACC_EXPR_KIND_VARIABLE;
    variable->name = name;
    return (WaccExpression*)variable;
}

WaccExpression* wacc_expr_new_assignment(str name, WaccExpression* expr)
{
    WaccAssignmentExpression* assignment = malloc(sizeof(WaccAssignmentExpression));
    assignment->base.kind = WACC_EXPR_KIND_ASSIGNMENT;
    assignment->name = name;
    assignment->expr = expr;
    return (WaccExpression*)assignment;
}

WaccStatement* wacc_stmt_new_declare(str name, WaccExpression* initializer)
{
    WaccDeclareStatement* declare = malloc(sizeof(WaccDeclareStatement));
    declare->base.kind = WACC_STMT_KIND_DECLARE;
    declare->name = name;
    declare->initializer = initializer;
    return (WaccStatement*)declare;
}

WaccStatement* wacc_stmt_new_return(WaccExpression* expression)
{
    WaccReturnStatement* ret = malloc(sizeof(WaccReturnStatement));
    ret->base.kind = WACC_STMT_KIND_RETURN;
    ret->expression = expression;
    return (WaccStatement*)ret;
}

WaccStatement* wacc_stmt_new_expression(WaccExpression* expression)
{
    WaccExpressionStatement* expr = malloc(sizeof(WaccExpressionStatement));
    expr->base.kind = WACC_STMT_KIND_EXPRESSION;
    expr->expression = expression;
    return (WaccStatement*)expr;
}

void ast_show(WaccNode root, FILE* out, FILE* err)
{
    AstDumper d = {0};
    switch (root.kind)
    {
        case WACC_NODE_PROGRAM:
            show_program(&d, root.as.program, out);
            break;
        default:
            (void)fprintf(err, "That's not a root node!\n");
            exit(1);
    }
}

static void expression_free(WaccExpression* expression)
{
    if (expression == NULL)
    {
        return;
    }
    switch (expression->kind)
    {
        case WACC_EXPR_KIND_CONSTANT:
            break;
        case WACC_EXPR_KIND_UNARY:
            expression_free(((WaccUnaryExpression*)expression)->expr);
            break;
        case WACC_EXPR_KIND_BINARY:
            expression_free(((WaccBinaryExpression*)expression)->lhs);
            expression_free(((WaccBinaryExpression*)expression)->rhs);
            break;
        case WACC_EXPR_KIND_ASSIGNMENT:
            str_free(((WaccAssignmentExpression*)expression)->name);
            expression_free(((WaccAssignmentExpression*)expression)->expr);
            break;
        case WACC_EXPR_KIND_VARIABLE:
            str_free(((WaccVariableExpression*)expression)->name);
            break;
        default:
            // invalid, ignore
            break;
    }
    free(expression);
}

static void statement_free(WaccStatement* statement)
{
    if (statement == NULL)
    {
        return;
    }
    switch (statement->kind)
    {
        case WACC_STMT_KIND_DECLARE:
            expression_free(((WaccDeclareStatement*)statement)->initializer);
            break;
        case WACC_STMT_KIND_RETURN:
            expression_free(((WaccReturnStatement*)statement)->expression);
            break;
        case WACC_STMT_KIND_EXPRESSION:
            expression_free(((WaccExpressionStatement*)statement)->expression);
            break;
        default:
            // invalid, ignore
            break;
    }
    free(statement);
}

void ast_free(WaccNode root)
{
    switch (root.kind)
    {
        case WACC_NODE_PROGRAM:
            str_free(root.as.program.function.name);
            for (uint64_t i = 0; i < root.as.program.function.statements.len; i++)
            {
                statement_free(root.as.program.function.statements.ptr[i]);
            }
            BUF_FREE(root.as.program.function.statements);
            break;
        default:
            (void)fprintf(stderr, "That's not a root node!\n");
            exit(1);
    }
}
