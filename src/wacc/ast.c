#include "wacc/ast.h"

#include <inttypes.h>
#include <stdlib.h>

typedef struct
{
    uint64_t depth;
} AstDumper;

static void show_indent(AstDumper* dumper)
{
    for (uint64_t i = 0; i < dumper->depth; i++)
    {
        printf("  ");
    }
}

static void show_expression(AstDumper* d, WaccExpression expression)
{
    show_indent(d);
    printf("%" PRIu64 "\n", expression.value);
}

static void show_statement(AstDumper* d, WaccStatement statement)
{
    show_indent(d);
    printf("RETURN:\n");
    d->depth++;
    show_expression(d, statement.expression);
    d->depth--;
}

static void show_function(AstDumper* d, WaccFunction function)
{
    printf("FUNCTION " str_fmt ":\n", str_arg(function.name));
    d->depth++;
    show_statement(d, function.statement);
    d->depth--;
}

static void show_program(AstDumper* d, WaccProgram program)
{
    show_function(d, program.function);
}

void ast_show(WaccNode root)
{
    AstDumper d = {0};
    switch (root.kind)
    {
        case WACC_NODE_PROGRAM:
            show_program(&d, root.as.program);
            break;
        default:
            (void)fprintf(stderr, "That's not a root node!\n");
            exit(1);
    }
}

void ast_free(WaccNode root)
{
    switch (root.kind)
    {
        case WACC_NODE_PROGRAM:
            str_free(root.as.program.function.name);
            break;
        default:
            (void)fprintf(stderr, "That's not a root node!\n");
            exit(1);
    }
}
