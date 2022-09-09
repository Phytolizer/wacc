#include "wacc/test/collect.h"

#include "str/strtox.h"
#include "wacc/test/impl.h"

#include <config.h>
#include <dirent.h>

typedef struct
{
    bool is_valid_dir;
} TestInfo;

static void walk_stage_dir(TestCaseBuf* tests, TestInfo* info, str path)
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
        if (str_eq(name, str_lit(".")) || str_eq(name, str_lit("..")))
        {
            continue;
        }

        if (entry->d_type == DT_DIR)
        {
            str prefix = str_lit("stage_");
            if (str_has_prefix(name, prefix))
            {
                Str2U64Result stage = str2u64(str_after(name, str_len(prefix)), 10);
                if (stage.err != 0)
                {
                    continue;
                }
                if (stage.value > IMPLEMENTED_STAGES)
                {
                    continue;
                }
            }
            bool is_valid_dir = str_eq(name, str_lit("valid"));
            info->is_valid_dir = is_valid_dir;
            str child_path = str_null;
            path_join(&child_path, path, name);
            walk_stage_dir(tests, info, child_path);
            if (is_valid_dir)
            {
                info->is_valid_dir = false;
            }
            str_free(child_path);
        }
        else if (entry->d_type == DT_REG)
        {
            if (str_has_suffix(name, str_lit(".c")))
            {
                str child_path = str_null;
                path_join(&child_path, path, name);
                TestCase test = {
                    .path = child_path,
                    .valid = info->is_valid_dir,
                    .skip_on_failure = str_has_prefix(name, str_lit("skip_on_failure_")),
                };
                BUF_PUSH(tests, test);
            }
        }
    }

    closedir(dir);
}

TestCaseBuf collect_tests(void)
{
    TestCaseBuf tests = BUF_NEW;
    TestInfo info = {0};
    walk_stage_dir(&tests, &info, str_lit(PROJECT_SOURCE_DIR "/tests/cases"));
    return tests;
}
