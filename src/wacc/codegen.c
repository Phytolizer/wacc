#include "wacc/codegen.h"

#include <hedley.h>
#include <inttypes.h>
#include <stdarg.h>

typedef struct
{
    uint64_t depth;
    uint64_t next_label;
} CodeGenerator;

static void show_indent(CodeGenerator* gen, FILE* fp)
{
    for (uint64_t i = 0; i < gen->depth; i++)
    {
        (void)fprintf(fp, "    ");
    }
}

static str gen_label(CodeGenerator* gen, const char* name_template)
{
    return str_printf(name_template, gen->next_label++);
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

static void codegen_expression(CodeGenerator* gen, FILE* fp, WaccExpression* expression);

static void codegen_unary_expression(CodeGenerator* gen, FILE* fp, WaccUnaryExpression* unary)
{
    switch (unary->op)
    {
        case WACC_UNARY_OP_ARITHMETIC_NEGATION:
            codegen_expression(gen, fp, unary->expr);
            emit(gen, fp, "neg rax");
            break;
        case WACC_UNARY_OP_LOGICAL_NEGATION:
            codegen_expression(gen, fp, unary->expr);
            emit(gen, fp, "cmp rax, 0");
            emit(gen, fp, "sete al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_UNARY_OP_BITWISE_NEGATION:
            codegen_expression(gen, fp, unary->expr);
            emit(gen, fp, "not rax");
            break;
        default:
            (void)fprintf(fp, "unknown unary operation");
            break;
    }
}

static void codegen_constant_expression(CodeGenerator* gen, FILE* fp, WaccConstantExpression* constant)
{
    emit(gen, fp, "mov rax, %" PRIu64, constant->value);
}

static void codegen_binary_expression(CodeGenerator* gen, FILE* fp, WaccBinaryExpression* binary)
{
    switch (binary->op)
    {
        case WACC_BINARY_OP_ADDITION:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "add rax, rsi");
            break;
        case WACC_BINARY_OP_SUBTRACTION:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "sub rsi, rax");
            emit(gen, fp, "mov rax, rsi");
            break;
        case WACC_BINARY_OP_MULTIPLICATION:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "imul rsi");
            break;
        case WACC_BINARY_OP_DIVISION:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "mov rdi, rax");
            emit(gen, fp, "mov rax, rsi");
            emit(gen, fp, "cqo");
            emit(gen, fp, "idiv rdi");
            break;
        case WACC_BINARY_OP_LOGICAL_AND: {
            str true_label = gen_label(gen, ".Ltrue%" PRIu64);
            str end_label = gen_label(gen, ".Lend%" PRIu64);
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "cmp rax, 0");
            emit(gen, fp, "jne " str_fmt, str_arg(true_label));
            emit(gen, fp, "jmp " str_fmt, str_arg(end_label));
            emit_label(fp, str_fmt, str_arg(true_label));
            str_free(true_label);
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "cmp rax, 0");
            emit(gen, fp, "setne al");
            emit(gen, fp, "movzx rax, al");
            emit_label(fp, str_fmt, str_arg(end_label));
            str_free(end_label);
            break;
        }
        case WACC_BINARY_OP_LOGICAL_OR: {
            str false_label = gen_label(gen, ".Lfalse%" PRIu64);
            str end_label = gen_label(gen, ".Lend%" PRIu64);
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "cmp rax, 0");
            emit(gen, fp, "je " str_fmt, str_arg(false_label));
            emit(gen, fp, "jmp " str_fmt, str_arg(end_label));
            emit_label(fp, str_fmt, str_arg(false_label));
            str_free(false_label);
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "cmp rax, 0");
            emit(gen, fp, "setne al");
            emit(gen, fp, "movzx rax, al");
            emit_label(fp, str_fmt, str_arg(end_label));
            str_free(end_label);
            break;
        }
        case WACC_BINARY_OP_EQUALITY:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "cmp rax, rsi");
            emit(gen, fp, "sete al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_BINARY_OP_INEQUALITY:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "cmp rax, rsi");
            emit(gen, fp, "setne al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_BINARY_OP_LESS:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "cmp rsi, rax");
            emit(gen, fp, "setl al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_BINARY_OP_LESS_EQUAL:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "cmp rsi, rax");
            emit(gen, fp, "setle al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_BINARY_OP_GREATER:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "cmp rsi, rax");
            emit(gen, fp, "setg al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_BINARY_OP_GREATER_EQUAL:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "cmp rsi, rax");
            emit(gen, fp, "setge al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_BINARY_OP_MODULO:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "mov rdi, rax");
            emit(gen, fp, "mov rax, rsi");
            emit(gen, fp, "cqo");
            emit(gen, fp, "idiv rdi");
            emit(gen, fp, "mov rax, rdx");
            break;
        case WACC_BINARY_OP_BITWISE_AND:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "and rax, rsi");
            break;
        case WACC_BINARY_OP_BITWISE_OR:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "or rax, rsi");
            break;
        case WACC_BINARY_OP_BITWISE_XOR:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "xor rax, rsi");
            break;
        case WACC_BINARY_OP_BITWISE_LSHIFT:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rdi");
            emit(gen, fp, "mov rcx, rax");
            emit(gen, fp, "mov rax, rdi");
            emit(gen, fp, "sal rax, cl");
            break;
        case WACC_BINARY_OP_BITWISE_RSHIFT:
            codegen_expression(gen, fp, binary->lhs);
            emit(gen, fp, "push rax");
            codegen_expression(gen, fp, binary->rhs);
            emit(gen, fp, "pop rdi");
            emit(gen, fp, "mov rcx, rax");
            emit(gen, fp, "mov rax, rdi");
            emit(gen, fp, "sar rax, cl");
            break;
    }
}

static void codegen_expression(CodeGenerator* gen, FILE* fp, WaccExpression* expression)
{
    switch (expression->kind)
    {
        case WACC_EXPR_KIND_UNARY:
            codegen_unary_expression(gen, fp, (WaccUnaryExpression*)expression);
            break;
        case WACC_EXPR_KIND_CONSTANT:
            codegen_constant_expression(gen, fp, (WaccConstantExpression*)expression);
            break;
        case WACC_EXPR_KIND_BINARY:
            codegen_binary_expression(gen, fp, (WaccBinaryExpression*)expression);
            break;
        default:
            HEDLEY_UNREACHABLE();
    }
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
    CodeGenerator gen = {.depth = 1, .next_label = 0};
    emit(&gen, fp, "section .text");
    emit(&gen, fp, "global _start");
    emit_label(fp, "_start");
    emit(&gen, fp, "call main");
    emit(&gen, fp, "mov rdi, rax");
    emit(&gen, fp, "mov rax, 60");
    emit(&gen, fp, "syscall");
    codegen_function(&gen, fp, program.function);
}
