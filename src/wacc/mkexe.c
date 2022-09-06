#include "wacc/mkexe.h"

#include "wacc/codegen.h"

#include <buf/buf.h>
#include <dirent.h>
#include <process/process.h>
#include <stdlib.h>
#include <unistd.h>

#define PROCESS_ARGS(...) (ProcessCStrBuf) BUF_ARRAY(((const char*[]){__VA_ARGS__}))

#define path_join(dest, ...) str_join((dest), str_lit("/"), __VA_ARGS__)

static void del_dir(str path)
{
    DIR* dir = opendir(path.ptr);
    if (dir == NULL)
    {
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        str name = str_ref(entry->d_name);
        if (entry->d_type == DT_DIR)
        {
            if (str_eq(name, str_lit(".")) || str_eq(name, str_lit("..")))
            {
                continue;
            }
            str next = str_null;
            path_join(&next, path, name);
            del_dir(next);
        }
        else
        {
            str child = str_null;
            path_join(&child, path, name);
            unlink(child.ptr);
            str_free(child);
        }
    }

    closedir(dir);
    rmdir(path.ptr);
    str_free(path);
}

void mkexe(WaccProgram program, str exe_output)
{
    char temp_template[] = "wacc-XXXXXX";
    char* temp_dir = mkdtemp(temp_template);
    str asm_source = str_null;
    path_join(&asm_source, str_ref(temp_dir), str_lit("wacc.s"));
    FILE* asm_fp = fopen(asm_source.ptr, "w");
    codegen_program(program, asm_fp);
    (void)fclose(asm_fp);

    str obj_path = str_null;
    path_join(&obj_path, str_ref(temp_dir), str_lit("wacc.o"));
    ProcessCreateResult nasm_result =
        process_run(PROCESS_ARGS("nasm", "-f", "elf64", "-o", obj_path.ptr, asm_source.ptr),
            PROCESS_OPTION_COMBINED_STDOUT_STDERR | PROCESS_OPTION_SEARCH_USER_PATH);
    if (!nasm_result.present)
    {
        (void)fprintf(stderr, "Failed to run nasm\n");
        del_dir(str_ref(temp_dir));
        exit(1);
    }
    process_destroy(&nasm_result.value);

    str_free(asm_source);

    ProcessCreateResult ld_result = process_run(PROCESS_ARGS("ld", "-o", exe_output.ptr, obj_path.ptr),
        PROCESS_OPTION_COMBINED_STDOUT_STDERR | PROCESS_OPTION_SEARCH_USER_PATH);
    if (!ld_result.present)
    {
        (void)fprintf(stderr, "Failed to run ld\n");
        del_dir(str_ref(temp_dir));
        exit(1);
    }
    process_destroy(&ld_result.value);

    str_free(obj_path);
    del_dir(str_ref(temp_dir));
}
