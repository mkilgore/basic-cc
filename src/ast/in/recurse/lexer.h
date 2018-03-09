#ifndef INCLUDE_RECURSE_LEXER_H
#define INCLUDE_RECURSE_LEXER_H

#include <stdio.h>
#include "ast/ast.h"

struct tok_location {
    off_t file_line_offset; /* Byte offset of the beginning of the line containing this token. */
    int first_line;
    int first_column;
    int last_line;
    int last_column;
};

struct lextok {
    int tok;
    union {
        char *str;
        int ival;
    };
    struct tok_location location;

    list_node_t entry;
};

#define LEXTOK_INIT(e) \
    { \
        .entry = LIST_NODE_INIT((e).entry), \
    }

static inline void lextok_init(struct lextok *t)
{
    *t = (struct lextok)LEXTOK_INIT(*t);
}

struct bcc_lexer_state {
    off_t file_offset;
    char *lex_str;
    size_t lex_str_capacity;
    size_t lex_str_len;

    struct tok_location cur_tok;
};

#define RECURSE_DECL \
    void recurse_lex(struct bcc_ast *ast, struct bcc_lexer_state *state, struct lextok *curtok)

RECURSE_DECL;

int recurse_lex_destroy(void);

extern FILE *recurse_in;

#endif
