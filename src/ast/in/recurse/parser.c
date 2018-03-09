
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container_of.h"
#include "list.h"

#include "parser.h"
#include "lexer.h"

#include "ast.h"

int parse_function(struct recurse_parse_state *state)
{
    struct lextok *tok = parser_peek(state);

}

int recurse_parser(struct recurse_parse_state *state)
{
    while (1) {
        struct lextok *tok = parser_peek(state);

        if (parse_function(state) == 0)
            continue;

        if (tok->tok == TOK_EOF)
            return 0;
    }
}

struct lextok *parser_peek(struct recurse_parse_state *state)
{
    if (state->token_is_valid)
        return &state->next_token;

    lextok_init(&state->next_token);
    recurse_lex(state->ast, &state->lex_state, &state->next_token);

    return &state->next_token;
}

void parser_consume(struct recurse_parse_state *state)
{
    state->token_is_valid = 0;
}

void parser_invalid_token_error(struct recurse_parse_state *state)
{
    struct lextok *tok = &state->next_token;
    FILE *in = state->in;

    printf("Error: Unexpected Token:\n");

    off_t sav = ftell(in);

    fseek(in, tok->location.file_line_offset, SEEK_SET);

    char *buf = NULL;
    size_t len = 0;
    getline(&buf, &len, in);

    printf("%s", buf);

    int i;
    for (i = 0; i < tok->location.first_column; i++)
        putchar(' ');

    putchar('^');
    i++;

    for (; i < tok->location.last_column; i++)
        putchar('-');

    if (tok->location.last_column > tok->location.first_column)
        putchar('^');

    putchar('\n');

    fseek(in, sav, SEEK_SET);
}

