#include "wacc/codegen.h"

#include <hedley.h>
#include <inttypes.h>
#include <stdarg.h>
#include <sum/sum.h>
#include <uthash.h>

typedef struct
{
    str name;
    uint64_t offset;
    UT_hash_handle hh;
} Frame;

typedef BUF(Frame*) FrameStack;

typedef struct
{
    uint64_t depth;
    uint64_t next_label;
    FrameStack frames;
    int64_t stack_index;
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

static void put_var(CodeGenerator* gen, str name, int64_t offset)
{
    Frame* frame = malloc(sizeof(Frame));
    frame->name = name;
    frame->offset = offset;
    HASH_ADD_KEYPTR(hh, BUF_TOP(&gen->frames), frame->name.ptr, frame->name.len, frame);
}

typedef RESULT(uint64_t, str) GetVarResult;

static GetVarResult get_var_rec(CodeGenerator* gen, str name, uint64_t depth)
{
    Frame* frame;
    HASH_FIND(hh, gen->frames.ptr[depth], name.ptr, name.len, frame);
    if (frame)
    {
        return (GetVarResult)OK(frame->offset);
    }
    if (depth > 0)
    {
        return get_var_rec(gen, name, depth - 1);
    }

    return (GetVarResult)ERR(str_printf("variable " str_fmt " not declared", str_arg(name)));
}

static GetVarResult get_var(CodeGenerator* gen, str name)
{
    Frame* frame;
    HASH_FIND(hh, BUF_TOP(&gen->frames), name.ptr, name.len, frame);
    if (frame)
    {
        return (GetVarResult)OK(frame->offset);
    }

    if (gen->frames.len > 1)
    {
        return get_var_rec(gen, name, gen->frames.len - 2);
    }

    return (GetVarResult)ERR(str_printf("variable " str_fmt " not declared", str_arg(name)));
}

static Frame* vars(CodeGenerator* gen)
{
    return BUF_TOP(&gen->frames);
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

HEDLEY_WARN_UNUSED_RESULT static CodegenError codegen_expression(
    CodeGenerator* gen, FILE* fp, WaccExpression* expression);

#define TRY(call) \
    do \
    { \
        CodegenError err = call; \
        if (err.present) \
        { \
            return err; \
        } \
    } while (0)

HEDLEY_WARN_UNUSED_RESULT static CodegenError codegen_unary_expression(
    CodeGenerator* gen, FILE* fp, WaccUnaryExpression* unary)
{
    switch (unary->op)
    {
        case WACC_UNARY_OP_ARITHMETIC_NEGATION:
            TRY(codegen_expression(gen, fp, unary->expr));
            emit(gen, fp, "neg rax");
            break;
        case WACC_UNARY_OP_LOGICAL_NEGATION:
            TRY(codegen_expression(gen, fp, unary->expr));
            emit(gen, fp, "cmp rax, 0");
            emit(gen, fp, "sete al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_UNARY_OP_BITWISE_NEGATION:
            TRY(codegen_expression(gen, fp, unary->expr));
            emit(gen, fp, "not rax");
            break;
        default:
            (void)fprintf(fp, "unknown unary operation");
            break;
    }
    return (CodegenError)NOTHING;
}

HEDLEY_WARN_UNUSED_RESULT static CodegenError codegen_constant_expression(
    CodeGenerator* gen, FILE* fp, WaccConstantExpression* constant)
{
    emit(gen, fp, "mov rax, %" PRIu64, constant->value);
    return (CodegenError)NOTHING;
}

HEDLEY_WARN_UNUSED_RESULT static CodegenError codegen_binary_expression(
    CodeGenerator* gen, FILE* fp, WaccBinaryExpression* binary)
{
    switch (binary->op)
    {
        case WACC_BINARY_OP_ADDITION:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "add rax, rsi");
            break;
        case WACC_BINARY_OP_SUBTRACTION:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "sub rsi, rax");
            emit(gen, fp, "mov rax, rsi");
            break;
        case WACC_BINARY_OP_MULTIPLICATION:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "imul rsi");
            break;
        case WACC_BINARY_OP_DIVISION:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "mov rdi, rax");
            emit(gen, fp, "mov rax, rsi");
            emit(gen, fp, "cqo");
            emit(gen, fp, "idiv rdi");
            break;
        case WACC_BINARY_OP_LOGICAL_AND: {
            str true_label = gen_label(gen, ".Ltrue%" PRIu64);
            str end_label = gen_label(gen, ".Lend%" PRIu64);
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "cmp rax, 0");
            emit(gen, fp, "jne " str_fmt, str_arg(true_label));
            emit(gen, fp, "jmp " str_fmt, str_arg(end_label));
            emit_label(fp, str_fmt, str_arg(true_label));
            str_free(true_label);
            TRY(codegen_expression(gen, fp, binary->rhs));
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
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "cmp rax, 0");
            emit(gen, fp, "je " str_fmt, str_arg(false_label));
            emit(gen, fp, "jmp " str_fmt, str_arg(end_label));
            emit_label(fp, str_fmt, str_arg(false_label));
            str_free(false_label);
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "cmp rax, 0");
            emit(gen, fp, "setne al");
            emit(gen, fp, "movzx rax, al");
            emit_label(fp, str_fmt, str_arg(end_label));
            str_free(end_label);
            break;
        }
        case WACC_BINARY_OP_EQUALITY:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "cmp rax, rsi");
            emit(gen, fp, "sete al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_BINARY_OP_INEQUALITY:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "cmp rax, rsi");
            emit(gen, fp, "setne al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_BINARY_OP_LESS:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "cmp rsi, rax");
            emit(gen, fp, "setl al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_BINARY_OP_LESS_EQUAL:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "cmp rsi, rax");
            emit(gen, fp, "setle al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_BINARY_OP_GREATER:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "cmp rsi, rax");
            emit(gen, fp, "setg al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_BINARY_OP_GREATER_EQUAL:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "cmp rsi, rax");
            emit(gen, fp, "setge al");
            emit(gen, fp, "movzx rax, al");
            break;
        case WACC_BINARY_OP_MODULO:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "mov rdi, rax");
            emit(gen, fp, "mov rax, rsi");
            emit(gen, fp, "cqo");
            emit(gen, fp, "idiv rdi");
            emit(gen, fp, "mov rax, rdx");
            break;
        case WACC_BINARY_OP_BITWISE_AND:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "and rax, rsi");
            break;
        case WACC_BINARY_OP_BITWISE_OR:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "or rax, rsi");
            break;
        case WACC_BINARY_OP_BITWISE_XOR:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rsi");
            emit(gen, fp, "xor rax, rsi");
            break;
        case WACC_BINARY_OP_BITWISE_LSHIFT:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rdi");
            emit(gen, fp, "mov rcx, rax");
            emit(gen, fp, "mov rax, rdi");
            emit(gen, fp, "sal rax, cl");
            break;
        case WACC_BINARY_OP_BITWISE_RSHIFT:
            TRY(codegen_expression(gen, fp, binary->lhs));
            emit(gen, fp, "push rax");
            TRY(codegen_expression(gen, fp, binary->rhs));
            emit(gen, fp, "pop rdi");
            emit(gen, fp, "mov rcx, rax");
            emit(gen, fp, "mov rax, rdi");
            emit(gen, fp, "sar rax, cl");
            break;
    }
    return (CodegenError)NOTHING;
}

HEDLEY_WARN_UNUSED_RESULT static CodegenError codegen_assignment_expression(
    CodeGenerator* gen, FILE* fp, WaccAssignmentExpression* assign)
{
    TRY(codegen_expression(gen, fp, assign->expr));
    GetVarResult var = get_var(gen, assign->name);
    if (!var.ok)
    {
        return (CodegenError)JUST(var.get.error);
    }
    emit(gen, fp, "mov [rbp-%" PRIu64 "], rax", var.get.value);
    return (CodegenError)NOTHING;
}

HEDLEY_WARN_UNUSED_RESULT static CodegenError codegen_variable_expression(
    CodeGenerator* gen, FILE* fp, WaccVariableExpression* var)
{
    GetVarResult var_result = get_var(gen, var->name);
    if (!var_result.ok)
    {
        return (CodegenError)JUST(var_result.get.error);
    }
    emit(gen, fp, "mov rax, [rbp-%" PRIu64 "]", var_result.get.value);
    return (CodegenError)NOTHING;
}

HEDLEY_WARN_UNUSED_RESULT static CodegenError codegen_expression(
    CodeGenerator* gen, FILE* fp, WaccExpression* expression)
{
    switch (expression->kind)
    {
        case WACC_EXPR_KIND_UNARY:
            TRY(codegen_unary_expression(gen, fp, (WaccUnaryExpression*)expression));
            break;
        case WACC_EXPR_KIND_CONSTANT:
            TRY(codegen_constant_expression(gen, fp, (WaccConstantExpression*)expression));
            break;
        case WACC_EXPR_KIND_BINARY:
            TRY(codegen_binary_expression(gen, fp, (WaccBinaryExpression*)expression));
            break;
        case WACC_EXPR_KIND_ASSIGNMENT:
            TRY(codegen_assignment_expression(gen, fp, (WaccAssignmentExpression*)expression));
            break;
        case WACC_EXPR_KIND_VARIABLE:
            TRY(codegen_variable_expression(gen, fp, (WaccVariableExpression*)expression));
            break;
        default:
            HEDLEY_UNREACHABLE();
    }
    return (CodegenError)NOTHING;
}

HEDLEY_WARN_UNUSED_RESULT static CodegenError codegen_return_statement(
    CodeGenerator* gen, FILE* fp, WaccReturnStatement* statement)
{
    TRY(codegen_expression(gen, fp, statement->expression));
    emit(gen, fp, "mov rsp, rbp");
    emit(gen, fp, "pop rbp");
    emit(gen, fp, "ret");
    return (CodegenError)NOTHING;
}

HEDLEY_WARN_UNUSED_RESULT static CodegenError codegen_expression_statement(
    CodeGenerator* gen, FILE* fp, WaccExpressionStatement* statement)
{
    TRY(codegen_expression(gen, fp, statement->expression));
    return (CodegenError)NOTHING;
}

HEDLEY_WARN_UNUSED_RESULT static CodegenError codegen_declare_statement(
    CodeGenerator* gen, FILE* fp, WaccDeclareStatement* statement)
{
    {
        // check var already exists
        Frame* frame = vars(gen);
        Frame* found = NULL;
        HASH_FIND(hh, frame, statement->name.ptr, statement->name.len, found);
        if (found != NULL)
        {
            return (CodegenError)JUST(
                str_printf("error: variable '" str_fmt "' already declared in this scope\n", str_arg(statement->name)));
        }
    }
    if (statement->initializer)
    {
        TRY(codegen_expression(gen, fp, statement->initializer));
    }
    else
    {
        emit(gen, fp, "mov rax, 0");
    }
    emit(gen, fp, "push rax");
    put_var(gen, statement->name, gen->stack_index);
    gen->stack_index += sizeof(long);
    return (CodegenError)NOTHING;
}

HEDLEY_WARN_UNUSED_RESULT static CodegenError codegen_statement(CodeGenerator* gen, FILE* fp, WaccStatement* statement)
{
    switch (statement->kind)
    {
        case WACC_STMT_KIND_RETURN:
            TRY(codegen_return_statement(gen, fp, (WaccReturnStatement*)statement));
            break;
        case WACC_STMT_KIND_EXPRESSION:
            TRY(codegen_expression_statement(gen, fp, (WaccExpressionStatement*)statement));
            break;
        case WACC_STMT_KIND_DECLARE:
            TRY(codegen_declare_statement(gen, fp, (WaccDeclareStatement*)statement));
            break;
        default:
            abort();
            HEDLEY_UNREACHABLE();
    }
    return (CodegenError)NOTHING;
}

HEDLEY_WARN_UNUSED_RESULT static CodegenError codegen_function(CodeGenerator* gen, FILE* fp, WaccFunction function)
{
    emit_label(fp, str_fmt, str_arg(function.name));
    emit(gen, fp, "push rbp");
    emit(gen, fp, "mov rbp, rsp");
    for (uint64_t i = 0; i < function.statements.len; i++)
    {
        TRY(codegen_statement(gen, fp, function.statements.ptr[i]));
    }
    emit(gen, fp, "mov rsp, rbp");
    emit(gen, fp, "pop rbp");
    emit(gen, fp, "ret");
    return (CodegenError)NOTHING;
}

HEDLEY_WARN_UNUSED_RESULT CodegenError codegen_program(WaccProgram program, FILE* fp)
{
    CodeGenerator gen = {.depth = 1, .next_label = 0, .stack_index = sizeof(long)};
    BUF_PUSH(&gen.frames, NULL);
    emit(&gen, fp, "section .text");
    emit(&gen, fp, "global _start");
    emit_label(fp, "_start");
    emit(&gen, fp, "call main");
    emit(&gen, fp, "mov rdi, rax");
    emit(&gen, fp, "mov rax, 60");
    emit(&gen, fp, "syscall");
    TRY(codegen_function(&gen, fp, program.function));
    return (CodegenError)NOTHING;
}
