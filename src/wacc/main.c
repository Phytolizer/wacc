#include "embedded/grammar.yaep.h"

#include <stdbool.h>
#include <stdio.h>
#include <yaep.h>

int main()
{
    struct grammar* g = yaep_create_grammar();
    if (yaep_parse_grammar(g, true, GRAMMAR_YAEP) != 0)
    {
        (void)fprintf(stderr, "Failed to parse grammar\n");
        return 1;
    }
    printf("Hello, World!\n");
}
