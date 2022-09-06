%define api.header.include {"wacc/generated/c.bison.h"}
%define api.prefix {c_frontend_}
%define api.pure full
%define api.token.prefix {TT_}
%define parse.error detailed
%define parse.lac full
%define parse.trace
%header
%locations

%token KW_INT
%token KW_RETURN
%token IDENT
%token NUMBER
%token OPEN_PAREN
%token CLOSE_PAREN
%token SEMICOLON
%token OPEN_BRACE
%token CLOSE_BRACE

%%

program: function { $$ = $1; }

function:
    KW_INT
    IDENT
    OPEN_PAREN
    CLOSE_PAREN
    OPEN_BRACE
    statement
    CLOSE_BRACE
    { $$ = $1; }

statement:
    KW_RETURN
    expression
    SEMICOLON
    { $$ = $1; }

expression:
    NUMBER
    { $$ = $1; }
