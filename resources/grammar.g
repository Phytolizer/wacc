{
#include "str/str.h"
#include "wacc/ast.h"
#define D_ParseNode_User WaccNode
}

program: function
{
  $$.kind = WACC_NODE_PROGRAM;
  $$.as.program.function = $0.as.function;
};

function: 'int' IDENT '(' ')' '{' statement '}'
{
  $$.kind = WACC_NODE_FUNCTION;
  $$.as.function.name = str_null;
  str_cpy(&($$.as.function.name), str_ref($n1.start_loc.s));
  $$.as.function.statement = $5.as.statement;
};

statement: 'return' expression ';'
{
  $$.kind = WACC_NODE_STATEMENT;
  $$.as.statement.expression = $1.as.expression;
};

expression: NUMBER
{
  $$.kind = WACC_NODE_EXPRESSION;
  str num_str = str_ref($n0.start_loc.s);
  $$.as.expression.value = strtol(num_str.ptr, NULL, 10);
};

IDENT: "[a-zA-Z_][a-zA-Z0-9_]*";

NUMBER: "[0-9]+";

whitespace: "([ \t\n\r]|(%[^\n\r]*))*";
