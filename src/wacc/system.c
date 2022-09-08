#include "wacc/system.h"

typedef struct
{
    size_t line;
    size_t col;
} LineCol;

static LineCol get_line_col(WaccSystem* system, size_t pos)
{
    size_t i;
    for (i = 1; i < system->source.line_starts.len; i++)
    {
        if (pos < system->source.line_starts.ptr[i])
        {
            break;
        }
    }
    return (LineCol){
        .line = i + 1,
        .col = pos - system->source.line_starts.ptr[i - 1] + 1,
    };
}

WaccSystem* wacc_system_new(void)
{
    WaccSystem* system = malloc(sizeof(WaccSystem));
    system->source.path = str_null;
    system->source.fp = NULL;
    system->source.text = (Text)BUF_NEW;
    system->source.line_starts = (LineStartBuf)BUF_NEW;
    system->source.num_errors = 0;
    return system;
}

void wacc_system_free(WaccSystem* system)
{
    if (system->source.fp != NULL)
    {
        (void)fclose(system->source.fp);
    }
    str_free(system->source.path);
    BUF_FREE(system->source.line_starts);
    BUF_FREE(system->source.text);
    free(system);
}

void wacc_system_open_file(WaccSystem* system, str path)
{
    str_cpy(&system->source.path, path);
    system->source.fp = fopen(system->source.path.ptr, "r");
    if (system->source.fp == NULL)
    {
        (void)fprintf(stderr, "error: failed to open file '" str_fmt "'\n", str_arg(path));
        exit(1);
    }
}

int wacc_system_read_source(WaccSystem* system)
{
    int c = fgetc(system->source.fp);
    if (c != EOF)
    {
        BUF_PUSH(&system->source.text, (char)c);
        if (c == '\n')
        {
            BUF_PUSH(&system->source.line_starts, system->source.text.len);
        }
    }
    else if (ferror(system->source.fp))
    {
        (void)fprintf(stderr, "error: failed to read file '" str_fmt "'\n", str_arg(system->source.path));
        exit(1);
    }
    return c;
}

void wacc_system_handle_error(WaccSystem* system, ErrorKind error, Range range)
{
    system->source.num_errors++;
    LineCol line_col = get_line_col(system, range.start);
    switch (error)
    {
#define X(x) \
    case ERROR_##x: \
        (void)fprintf( \
            stderr, str_fmt ":%zu:%zu: error: " #x "\n", str_arg(system->source.path), line_col.line, line_col.col); \
        break;
#include "wacc/system/errors.def"
#undef X
    }
}
