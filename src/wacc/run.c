#include "wacc/run.h"

#include "wacc/ast.h"
#include "wacc/mkexe.h"

#include <arg/arg.h>
#include <dparse.h>
#include <stdbool.h>
#include <stdio.h>

extern D_ParserTables parser_tables_wacc;

int run(int argc, char** argv)
{
    Arg help_arg =
        ARG_FLAG(.shortname = 'h', .longname = arg_str_lit("help"), .help = arg_str_lit("Show this help message"));
    Arg* args[] = {&help_arg};
    ArgParser parser =
        arg_parser_new(arg_str_lit("wacc"), arg_str_lit("What A C Compiler!"), (ArgBuf)ARG_BUF_ARRAY(args));
    ArgParseErr arg_parse_err = arg_parser_parse(&parser, argc, argv);
    if (arg_parse_err.present)
    {
        arg_parser_show_help(&parser, stderr);
        (void)fprintf(stderr, "ERROR: " ARG_STR_FMT "\n", ARG_STR_ARG(arg_parse_err.value));
        arg_str_free(arg_parse_err.value);
        return 1;
    }
    if (help_arg.flagValue)
    {
        arg_parser_show_help(&parser, stdout);
        return 0;
    }
    D_Parser* p = new_D_Parser(&parser_tables_wacc, sizeof(WaccNode));
    p->save_parse_tree = true;
    str test_input = str_lit("int main() { return 2; }");
    D_ParseNode* pn = dparse(p, (char*)test_input.ptr, (int)test_input.len);
    if (pn != NULL)
    {
        printf("Parsed successfully\n");
        ast_show(pn->user);
        mkexe(pn->user.as.program, str_lit("test"));
        ast_free(pn->user);
        free_D_ParseNode(p, pn);
    }
    else
    {
        printf("Parse failed\n");
    }
    free_D_Parser(p);
    return 0;
}