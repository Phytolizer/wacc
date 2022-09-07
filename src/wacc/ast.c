#include "wacc/ast.h"

#include <hedley.h>
#include <inttypes.h>
#include <stdlib.h>

static const char* UNARY_OP_STRINGS[] = {
#define X(x) #x,
#include "wacc/ast_unary_op_kinds.def"
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
        default: {
            (void)fprintf(out, "unknown expression");
            break;
        }
    }
}

static void show_statement(AstDumper* d, WaccStatement statement, FILE* out)
{
    show_indent(d, out);
    (void)fprintf(out, "RETURN:\n");
    d->depth++;
    show_indent(d, out);
    show_expression(statement.expression, out);
    d->depth--;
}

static void show_function(AstDumper* d, WaccFunction function, FILE* out)
{
    (void)fprintf(out, "FUNCTION " str_fmt ":\n", str_arg(function.name));
    d->depth++;
    show_statement(d, function.statement, out);
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
        default:
            HEDLEY_UNREACHABLE();
    }
    free(expression);
}

void ast_free(WaccNode root)
{
    switch (root.kind)
    {
        case WACC_NODE_PROGRAM:
            str_free(root.as.program.function.name);
            expression_free(root.as.program.function.statement.expression);
            break;
        default:
            (void)fprintf(stderr, "That's not a root node!\n");
            exit(1);
    }
}
