#include "wacc/ast.h"

#include <assert.h>
#include <stdlib.h>

WaccNode* wacc_node_new_text(str text)
{
    WaccNode* node = malloc(sizeof(WaccNode));
    node->kind = WACC_NODE_TEXT;
    node->as.text = text;
    return node;
}

WaccNode* wacc_node_new_program(WaccFunction function)
{
    WaccNode* node = malloc(sizeof(WaccNode));
    node->kind = WACC_NODE_PROGRAM;
    node->as.program.function = function;
    return node;
}

WaccNode* wacc_node_new_function(str name, WaccStatement statement)
{
    WaccNode* node = malloc(sizeof(WaccNode));
    node->kind = WACC_NODE_FUNCTION;
    node->as.function.name = name;
    node->as.function.statement = statement;
    return node;
}

WaccNode* wacc_node_new_statement(WaccExpression* expression)
{
    WaccNode* node = malloc(sizeof(WaccNode));
    node->kind = WACC_NODE_STATEMENT;
    node->as.statement.expression = expression;
    return node;
}

WaccNode* wacc_node_new_expression(uint64_t value)
{
    WaccNode* node = malloc(sizeof(WaccNode));
    node->kind = WACC_NODE_EXPRESSION;
    WaccConstantExpression* constant = malloc(sizeof(WaccConstantExpression));
    constant->base.type = WACC_EXPR_CONSTANT;
    constant->value = value;
    node->as.expression = (WaccExpression*)constant;
    return node;
}

WaccNode* wacc_error_node_expression(void)
{
    WaccNode* node = malloc(sizeof(WaccNode));
    node->kind = WACC_NODE_EXPRESSION;
    WaccExpression* error_expr = malloc(sizeof(WaccExpression));
    error_expr->type = WACC_EXPR_ERROR;
    node->as.expression = error_expr;
    return node;
}

static void expression_free(WaccExpression* expr)
{
    switch (expr->type)
    {
        case WACC_EXPR_CONSTANT:
        case WACC_EXPR_ERROR:
            free(expr);
            break;
        default:
            abort();
    }
}

static void function_free(WaccFunction function)
{
    str_free(function.name);
    expression_free(function.statement.expression);
}

void ast_free(WaccNode* ast)
{
    assert(ast->kind == WACC_NODE_PROGRAM);
    function_free(ast->as.program.function);
    free(ast);
}
