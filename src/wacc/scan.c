#include "wacc/scan.h"

#include <grammar.g.d_parser.h>
#include <stdbool.h>
#include <string.h>

static bool char_is_letter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool char_is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static bool char_is_letter_or_digit(char c)
{
    return char_is_letter(c) || char_is_digit(c);
}

static bool strlen_ge(const char* s, size_t n)
{
    size_t i = 0;
    while (s[i] != 0 && i < n)
    {
        i++;
    }
    return i == n;
}

int wacc_scan(d_loc_t* loc, unsigned short* symbol, int* term_priority, unsigned char* op_assoc, int* op_priority)
{
    (void)term_priority;
    (void)op_assoc;
    (void)op_priority;
    char* beg = loc->s;
    if (char_is_letter(*loc->s))
    {
        *symbol = TT_IDENT;
        while (char_is_letter_or_digit(*loc->s))
        {
            loc->s += 1;
        }
        if (loc->s - beg == 6 && strncmp(beg, "return", 6) == 0)
        {
            *symbol = KW_RETURN;
        }
        else if (loc->s - beg == 3 && strncmp(beg, "int", 3) == 0)
        {
            *symbol = KW_INT;
        }
        return 1;
    }
    return 0;
}
