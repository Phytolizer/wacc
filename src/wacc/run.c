#include "wacc/run.h"

#include "wacc/ast.h"
#include "wacc/mkexe.h"

#include <arg/arg.h>
#include <dparse.h>
#include <file/file.h>
#include <stdbool.h>
#include <stdio.h>

extern D_ParserTables parser_tables_wacc;

int run(Argv argv, FILE* out, FILE* err)
{
    Arg help_arg =
        ARG_FLAG(.shortname = 'h', .longname = arg_str_lit("help"), .help = arg_str_lit("Show this help message"));
    Arg output_arg = ARG_OPT(.shortname = 'o', .longname = arg_str_lit("output"), .help = arg_str_lit("Output file"));
    Arg file_arg = ARG_POS(arg_str_lit("FILE"), arg_str_lit("Input file"));
    Arg* args[] = {&help_arg, &output_arg, &file_arg};
    ArgParser parser =
        arg_parser_new(arg_str_lit("wacc"), arg_str_lit("What A C Compiler!"), (ArgBuf)ARG_BUF_ARRAY(args));
    ArgParseErr arg_parse_err = arg_parser_parse(&parser, (int)argv.len, argv.ptr);
    if (arg_parse_err.present)
    {
        arg_parser_show_help(&parser, stderr);
        (void)fprintf(err, "ERROR: " ARG_STR_FMT "\n", ARG_STR_ARG(arg_parse_err.value));
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
    SlurpFileResult test_input = slurp_file(str_ref_chars(file_arg.value.ptr, arg_str_len(file_arg.value)));
    if (!test_input.ok)
    {
        (void)fprintf(err, "Failed to read file: " str_fmt "\n", str_arg(test_input.get.error));
        str_free(test_input.get.error);
        return 1;
    }
    D_ParseNode* pn = dparse(p, (char*)test_input.get.value.ptr, (int)test_input.get.value.len);
    int ret = 0;
    if (pn != NULL && p->syntax_errors == 0)
    {
        (void)fprintf(out, "Parsed successfully\n");
        ast_show(pn->user, out, err);
        str output = str_lit("a.out");
        if (arg_str_len(output_arg.value) > 0)
        {
            output = str_ref_chars(output_arg.value.ptr, arg_str_len(output_arg.value));
        }
        mkexe(pn->user.as.program, output, err);
        ast_free(pn->user);
        free_D_ParseNode(p, pn);
    }
    else
    {
        if (pn != NULL)
        {
            ast_free(pn->user);
            free_D_ParseNode(p, pn);
        }
        (void)fprintf(out, "Parse failed\n");
        ret = 1;
    }
    str_free(test_input.get.value);
    free_D_Parser(p);
    ARG_BUF_FREE(parser.extra);
    return ret;
}
