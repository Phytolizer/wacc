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
    unary_op expression
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
};

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
};

IDENT: "[a-zA-Z_][a-zA-Z0-9_]*";

NUMBER: "[0-9]+";

whitespace: "[ \t\n\r]+";
