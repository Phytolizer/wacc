%option batch
%option nodefault
%option nounistd
%option noyywrap
%option yylineno
%option bison-bridge
%option bison-locations
%option reentrant
%option prefix="c_frontend_"

NUMBER [0-9]+
IDENT [a-zA-Z_][a-zA-Z0-9_]*
WS [ \t\r\n]+

%{
#include "wacc/generated/c.bison.h"
#define YYSTYPE C_FRONTEND_STYPE
#define YYLTYPE C_FRONTEND_LTYPE
%}

%%

{WS} ;
{NUMBER} { return TT_NUMBER; }
