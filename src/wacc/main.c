#include "wacc/ast.h"

#include <dparse.h>
#include <stdbool.h>
#include <stdio.h>

extern D_ParserTables parser_tables_wacc;

int main()
{
    D_Parser* p = new_D_Parser(&parser_tables_wacc, sizeof(WaccNode));
    p->save_parse_tree = true;
    str test_input = str_lit("int main() { return 2; }");
    D_ParseNode* pn = dparse(p, (char*)test_input.ptr, (int)test_input.len);
    if (pn != NULL)
    {
        printf("Parsed successfully\n");
        ast_show(pn->user);
        ast_free(pn->user);
        free_D_ParseNode(p, pn);
    }
    else
    {
        printf("Parse failed\n");
    }
    printf("Hello, World!\n");
    free_D_Parser(p);
}
