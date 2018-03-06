#ifndef SRC_AST_IN_PARSER_H
#define SRC_AST_IN_PARSER_H

#include "ast/ast.h"

struct bcc_parser_state {
    struct bae_block *current_scope;
    struct bae_function *current_func;

    list_head_t temp_param_list;

    char *lex_str;
    size_t lex_str_capacity;
    size_t lex_str_len;
};

#define BCC_PARSER_STATE_INIT(e) \
    { \
        .temp_param_list = LIST_HEAD_INIT((e).temp_param_list), \
    }

static inline void bcc_parser_state_init(struct bcc_parser_state *state)
{
    *state = (struct bcc_parser_state)BCC_PARSER_STATE_INIT(*state);
}

int yyparse(struct bcc_ast *, struct bcc_parser_state *);

#endif
