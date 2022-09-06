#include "wacc/codegen.h"

#include <hedley.h>
#include <inttypes.h>
#include <stdarg.h>

typedef struct
{
    uint64_t depth;
} CodeGenerator;

static void show_indent(CodeGenerator* gen, FILE* fp)
{
    for (uint64_t i = 0; i < gen->depth; i++)
    {
        (void)fprintf(fp, "    ");
    }
}

static void HEDLEY_PRINTF_FORMAT(3, 4) emit(CodeGenerator* gen, FILE* fp, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    show_indent(gen, fp);
    (void)vfprintf(fp, fmt, args);
    (void)fprintf(fp, "\n");
    va_end(args);
}

static void HEDLEY_PRINTF_FORMAT(2, 3) emit_label(FILE* fp, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    (void)vfprintf(fp, fmt, args);
    (void)fprintf(fp, ":\n");
    va_end(args);
}

static void codegen_expression(CodeGenerator* gen, FILE* fp, WaccExpression expression)
{
    emit(gen, fp, "mov rax, %" PRIu64, expression.value);
}

static void codegen_statement(CodeGenerator* gen, FILE* fp, WaccStatement statement)
{
    codegen_expression(gen, fp, statement.expression);
    emit(gen, fp, "ret");
}

static void codegen_function(CodeGenerator* gen, FILE* fp, WaccFunction function)
{
    emit_label(fp, str_fmt, str_arg(function.name));
    codegen_statement(gen, fp, function.statement);
}

void codegen_program(WaccProgram program, FILE* fp)
{
    CodeGenerator gen = {.depth = 1};
    emit(&gen, fp, "section .text");
    emit(&gen, fp, "global _start");
    emit_label(fp, "_start");
    emit(&gen, fp, "call main");
    emit(&gen, fp, "mov rdi, rax");
    emit(&gen, fp, "mov rax, 60");
    emit(&gen, fp, "syscall");
    codegen_function(&gen, fp, program.function);
}
