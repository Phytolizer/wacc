#include "wacc/run.h"

#include "packcc/grammar.h"
#include "wacc/ast.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

int run(WaccArgBuf args, FILE* out, FILE* err)
{
    (void)args;
    WaccSystem* sys = wacc_system_new();
    wacc_system_open_file(sys, str_lit("return_2.c"));
    wacc_context_t* ctx = wacc_create(sys);
    WaccNode* ast;
    if (wacc_parse(ctx, &ast) != 0)
    {
        (void)fprintf(err, "parse error\n");
        return 1;
    }
    if (sys->source.num_errors > 0)
    {
        (void)fprintf(err, "parse error\n");
        return 1;
    }
    ast_free(ast);
    wacc_system_free(sys);
    wacc_destroy(ctx);
    (void)fprintf(out, "Hello, World!\n");
    return 0;
}
