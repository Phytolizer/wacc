#include "packcc/grammar.h"
#include "wacc/ast.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

int main()
{
    WaccSystem* sys = wacc_system_new();
    wacc_system_open_file(sys, str_lit("return_2.c"));
    wacc_context_t* ctx = wacc_create(sys);
    WaccNode* ast;
    assert(wacc_parse(ctx, &ast) == 0);
    ast_free(ast);
    wacc_system_free(sys);
    wacc_destroy(ctx);
    printf("Hello, World!\n");
}
