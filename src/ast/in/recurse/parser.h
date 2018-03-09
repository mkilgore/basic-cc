#ifndef SRC_AST_IN_RECURSE_PARSE_H
#define SRC_AST_IN_RECURSE_PARSE_H

#include <stdint.h>
#include <stdio.h>
#include "ast.h"
#include "lexer.h"

struct recurse_parse_state {
    struct bcc_ast *ast;
    FILE *in;

    struct bae_block *current_scope;
    struct bae_function *current_func;

    uint8_t token_is_valid;
    struct lextok next_token;

    struct bcc_lexer_state lex_state;
};

#define RECURSE_PARSE_STATE_INIT() \
    { \
    }

static inline void recurse_parse_state_init(struct recurse_parse_state *state)
{
    *state = (struct recurse_parse_state)RECURSE_PARSE_STATE_INIT();
}

enum {
    TOK_IDENT = 258,
    TOK_FUNCTION = 259,
    TOK_STRING = 260,
    TOK_NUMBER = 261,
    TOK_NOT_EQUAL = 262,
    TOK_GREATER_THAN_EQUAL = 263,
    TOK_LESS_THAN_EQUAL = 264,
    TOK_EOF = 265,
    TOK_ERR = 266,
    TOK_TYPE = 267,
    TOK_RETURN = 268,
    TOK_WHILE = 269,
    TOK_IF = 270,
    TOK_ELSE = 271,
};

int recurse_parser_parse(struct recurse_parse_state *);
struct lextok *parser_get(struct recurse_parse_state *);
struct lextok *parser_peek(struct recurse_parse_state *);
void parser_consume(struct recurse_parse_state *);
void parser_invalid_token_error(struct recurse_parse_state *);

#endif
