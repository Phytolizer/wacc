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

#define IS_KW(s1, n, lit) (n == sizeof(lit) - 1 && memcmp(s1, lit, n) == 0)

int wacc_scan(d_loc_t* loc, unsigned short* symbol, int* term_priority, unsigned char* op_assoc, int* op_priority)
{
    (void)term_priority;
    (void)op_assoc;
    (void)op_priority;
    char* beg = loc->s;
    if (char_is_letter(*loc->s))
    {
        while (char_is_letter_or_digit(*loc->s))
        {
            loc->s += 1;
        }
        size_t len = loc->s - beg;
        if (IS_KW(beg, len, "return"))
        {
            *symbol = KW_RETURN;
        }
        else if (IS_KW(beg, len, "int"))
        {
            *symbol = KW_INT;
        }
        else
        {
            *symbol = TT_IDENT;
        }
        return 1;
    }
    return 0;
}
