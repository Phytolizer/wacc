#include "wacc/ast.h"

#include <inttypes.h>
#include <stdlib.h>

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

static void show_expression(AstDumper* d, WaccExpression expression, FILE* out)
{
    show_indent(d, out);
    (void)fprintf(out, "%" PRIu64 "\n", expression.value);
}

static void show_statement(AstDumper* d, WaccStatement statement, FILE* out)
{
    show_indent(d, out);
    (void)fprintf(out, "RETURN:\n");
    d->depth++;
    show_expression(d, statement.expression, out);
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
