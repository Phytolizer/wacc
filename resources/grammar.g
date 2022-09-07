{
#include "str/str.h"
#include "str/strtox.h"
#include "wacc/ast.h"
#include "wacc/scan.h"

#include <stdlib.h>
#include <string.h>

#define str_ref_node(node) str_ref_chars((node).start_loc.s, (node).end - (node).start_loc.s)
}

${scanner wacc_scan}
${token KW_RETURN KW_INT TT_IDENT}

program: function
{
  $$.kind = WACC_NODE_PROGRAM;
  $$.as.program.function = $0.as.function;
};

function: KW_INT TT_IDENT '(' ')' '{' statement '}'
{
  $$.kind = WACC_NODE_FUNCTION;
  $$.as.function.name = str_null;
  str_cpy(&($$.as.function.name), str_ref_node($n1));
  $$.as.function.statement = $5.as.statement;
};

statement: KW_RETURN expression ';'
{
  $$.kind = WACC_NODE_STATEMENT;
  $$.as.statement.expression = $1.as.expression;
};

expression:
  expression '||' logical_and_exp
    {
      $$.kind = WACC_NODE_EXPRESSION;
      $$.as.expression = wacc_expr_new_binary($0.as.expression, WACC_BINARY_OP_LOGICAL_OR, $2.as.expression);
    }
  | logical_and_exp
    {
      $$ = $0;
    }
  ;

logical_and_exp:
  logical_and_exp '&&' bitwise_or_exp
    {
      $$.kind = WACC_NODE_EXPRESSION;
      $$.as.expression = wacc_expr_new_binary($0.as.expression, WACC_BINARY_OP_LOGICAL_AND, $2.as.expression);
    }
  | bitwise_or_exp
    {
      $$ = $0;
    }
  ;

bitwise_or_exp:
  bitwise_or_exp '|' bitwise_xor_exp
    {
      $$.kind = WACC_NODE_EXPRESSION;
      $$.as.expression = wacc_expr_new_binary($0.as.expression, WACC_BINARY_OP_BITWISE_OR, $2.as.expression);
    }
  | bitwise_xor_exp
    {
      $$ = $0;
    }
  ;

bitwise_xor_exp:
  bitwise_xor_exp '^' bitwise_and_exp
    {
      $$.kind = WACC_NODE_EXPRESSION;
      $$.as.expression = wacc_expr_new_binary($0.as.expression, WACC_BINARY_OP_BITWISE_XOR, $2.as.expression);
    }
  | bitwise_and_exp
    {
      $$ = $0;
    }
  ;

bitwise_and_exp:
  bitwise_and_exp '&' equality_exp
    {
      $$.kind = WACC_NODE_EXPRESSION;
      $$.as.expression = wacc_expr_new_binary($0.as.expression, WACC_BINARY_OP_BITWISE_AND, $2.as.expression);
    }
  | equality_exp
    {
      $$ = $0;
    }
  ;

equality_exp:
  equality_exp equality_op relational_exp
    {
      $$.kind = WACC_NODE_EXPRESSION;
      $$.as.expression = wacc_expr_new_binary($0.as.expression, $1.as.binary_op, $2.as.expression);
    }
  | relational_exp
    {
      $$ = $0;
    }
  ;

relational_exp:
  relational_exp relational_op shift_exp
    {
      $$.kind = WACC_NODE_EXPRESSION;
      $$.as.expression = wacc_expr_new_binary($0.as.expression, $1.as.binary_op, $2.as.expression);
    }
  | shift_exp
    {
      $$ = $0;
    }
  ;

shift_exp:
  shift_exp shift_op additive_exp
    {
      $$.kind = WACC_NODE_EXPRESSION;
      $$.as.expression = wacc_expr_new_binary($0.as.expression, $1.as.binary_op, $2.as.expression);
    }
  | additive_exp
    {
      $$ = $0;
    }
  ;

additive_exp:
  additive_exp additive_op term
    {
      $$.kind = WACC_NODE_EXPRESSION;
      $$.as.expression = wacc_expr_new_binary($0.as.expression, $1.as.binary_op, $2.as.expression);
    }
  | term
    {
      $$ = $0;
    }
  ;

term:
  term multiplicative_op factor
    {
      $$.kind = WACC_NODE_EXPRESSION;
      $$.as.expression = wacc_expr_new_binary($0.as.expression, $1.as.binary_op, $2.as.expression);
    }
  | factor
    {
      $$ = $0;
    }
  ;

factor:
  '(' expression ')'
    {
      $$.kind = WACC_NODE_EXPRESSION;
      $$.as.expression = $1.as.expression;
    }
  | unary_op factor
    {
      $$.kind = WACC_NODE_EXPRESSION;
      $$.as.expression = wacc_expr_new_unary($0.as.unary_op, $1.as.expression);
    }
  | NUMBER
    {
      $$.kind = WACC_NODE_EXPRESSION;
      str num_str = str_ref_node($n0);
      Str2U64Result result = str2u64(num_str, 10);
      if (result.err)
      {
        fprintf(stderr, "Error: %s\n", strerror(result.err));
        exit(1);
      }
      $$.as.expression = wacc_expr_new_constant(result.value);
    }
  ;

unary_op:
  '-'
    {
      $$.kind = WACC_NODE_UNARY_OP;
      $$.as.unary_op = WACC_UNARY_OP_ARITHMETIC_NEGATION;
    }
  | '!'
    {
      $$.kind = WACC_NODE_UNARY_OP;
      $$.as.unary_op = WACC_UNARY_OP_LOGICAL_NEGATION;
    }
  | '~'
    {
      $$.kind = WACC_NODE_UNARY_OP;
      $$.as.unary_op = WACC_UNARY_OP_BITWISE_NEGATION;
    }
  ;

multiplicative_op:
  '*'
    {
      $$.kind = WACC_NODE_BINARY_OP;
      $$.as.binary_op = WACC_BINARY_OP_MULTIPLICATION;
    }
  | '/'
    {
      $$.kind = WACC_NODE_BINARY_OP;
      $$.as.binary_op = WACC_BINARY_OP_DIVISION;
    }
  | '%'
    {
      $$.kind = WACC_NODE_BINARY_OP;
      $$.as.binary_op = WACC_BINARY_OP_MODULO;
    }
  ;

additive_op:
  '+'
    {
      $$.kind = WACC_NODE_BINARY_OP;
      $$.as.binary_op = WACC_BINARY_OP_ADDITION;
    }
  | '-'
    {
      $$.kind = WACC_NODE_BINARY_OP;
      $$.as.binary_op = WACC_BINARY_OP_SUBTRACTION;
    }
  ;

equality_op:
  '=='
    {
      $$.kind = WACC_NODE_BINARY_OP;
      $$.as.binary_op = WACC_BINARY_OP_EQUALITY;
    }
  | '!='
    {
      $$.kind = WACC_NODE_BINARY_OP;
      $$.as.binary_op = WACC_BINARY_OP_INEQUALITY;
    }
  ;

relational_op:
  '<'
    {
      $$.kind = WACC_NODE_BINARY_OP;
      $$.as.binary_op = WACC_BINARY_OP_LESS;
    }
  | '>'
    {
      $$.kind = WACC_NODE_BINARY_OP;
      $$.as.binary_op = WACC_BINARY_OP_GREATER;
    }
  | '<='
    {
      $$.kind = WACC_NODE_BINARY_OP;
      $$.as.binary_op = WACC_BINARY_OP_LESS_EQUAL;
    }
  | '>='
    {
      $$.kind = WACC_NODE_BINARY_OP;
      $$.as.binary_op = WACC_BINARY_OP_GREATER_EQUAL;
    }
  ;

shift_op:
  '<<'
    {
      $$.kind = WACC_NODE_BINARY_OP;
      $$.as.binary_op = WACC_BINARY_OP_BITWISE_LSHIFT;
    }
  | '>>'
    {
      $$.kind = WACC_NODE_BINARY_OP;
      $$.as.binary_op = WACC_BINARY_OP_BITWISE_RSHIFT;
    }
  ;

IDENT: "[a-zA-Z_][a-zA-Z0-9_]*";

NUMBER: "[0-9]+";

whitespace: "[ \t\n\r]+";
