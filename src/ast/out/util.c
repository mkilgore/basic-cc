
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "util.h"

const char hex_table[] = {
    '0', '1', '2', '3', '4',
    '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F'
};

static char get_hex_char(int val)
{
    return hex_table[val];
}

static void append_char(char **str, int *cap, int *len, char ch)
{
    (*len)++;
    if (*len >= *cap) {
        *cap *= 2;
        *str = realloc(*str, *cap);
    }

    (*str)[*len - 1] = ch;
}

char *util_escape_str(const char *str)
{
    int capacity = 20;
    int len = 0;
    char *escstr = malloc(capacity);

    for (; *str; str++) {
        switch (*str) {
        case ' ' ... '\x7E':
            append_char(&escstr, &capacity, &len, *str);
            break;

        case '\n':
            append_char(&escstr, &capacity, &len, '\\');
            append_char(&escstr, &capacity, &len, 'n');
            break;

        case '\b':
            append_char(&escstr, &capacity, &len, '\\');
            append_char(&escstr, &capacity, &len, 'b');
            break;

        case '\r':
            append_char(&escstr, &capacity, &len, '\\');
            append_char(&escstr, &capacity, &len, 'r');
            break;

        case '\t':
            append_char(&escstr, &capacity, &len, '\\');
            append_char(&escstr, &capacity, &len, 't');
            break;

        default:
            append_char(&escstr, &capacity, &len, '\\');
            append_char(&escstr, &capacity, &len, 'x');
            append_char(&escstr, &capacity, &len, get_hex_char(*str >> 4));
            append_char(&escstr, &capacity, &len, get_hex_char(*str & 0x0F));
            break;
        }
    }

    append_char(&escstr, &capacity, &len, '\0');
    return escstr;
}

