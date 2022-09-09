#include "wacc/run.h"

#include "packcc/grammar.h"
#include "wacc/ast.h"

#include <arg/arg.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#define arg_str_to_str(arg_str) (str_ref_chars((arg_str).ptr, arg_str_len(arg_str)))

int run(WaccArgBuf args, FILE* out, FILE* err)
{
    Arg help_arg =
        ARG_FLAG(.shortname = 'h', .longname = arg_str_lit("help"), .help = arg_str_lit("Print this help message"));
    Arg file_arg = ARG_POS(arg_str_lit("FILE"), arg_str_lit("The file to compile"));
    Arg output_arg = ARG_OPT(.shortname = 'o', .longname = arg_str_lit("out"), .help = arg_str_lit("The output file"));
    Arg* supported_args[] = {&help_arg, &file_arg, &output_arg};
    ArgParser arg_parser = arg_parser_new(arg_str_lit("wacc"),
        arg_str_lit("What A C Compiler -- compile C programs to x86_64 ELF executable"),
        (ArgBuf)ARG_BUF_ARRAY(supported_args));
    ArgParseErr arg_parse_err = arg_parser_parse(&arg_parser, (int)args.len, args.ptr);
    if (help_arg.flagValue)
    {
        arg_parser_show_help(&arg_parser, out);
        return 0;
    }
    if (arg_parse_err.present)
    {
        arg_parser_show_help(&arg_parser, err);
        (void)fprintf(err, str_fmt "\n", str_arg(arg_str_to_str(arg_parse_err.value)));
        return 1;
    }
    WaccSystem* sys = wacc_system_new(err);
    if (wacc_system_open_file(sys, arg_str_to_str(file_arg.value), err) != 0)
    {
        return 1;
    }
    wacc_context_t* ctx = wacc_create(sys);
    WaccNode* ast;
    if (wacc_parse(ctx, &ast) != 0)
    {
        (void)fprintf(err, "parse error\n");
        wacc_system_free(sys);
        wacc_destroy(ctx);
        return 1;
    }
    if (sys->source.num_errors > 0)
    {
        (void)fprintf(err, "parse error\n");
        ast_free(ast);
        wacc_system_free(sys);
        wacc_destroy(ctx);
        return 1;
    }
    ast_free(ast);
    wacc_system_free(sys);
    wacc_destroy(ctx);
    (void)fprintf(out, "Hello, World!\n");
    return 0;
}
