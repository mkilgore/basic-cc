#ifndef INCLUDE_AST_NODES_LITERAL_NUMBER_H
#define INCLUDE_AST_NODES_LITERAL_NUMBER_H

#include <stdlib.h>
#include "ast/ast.h"

struct bae_literal_number {
    struct bcc_ast_entry ent;
    int value;
};

#define BAE_LITERAL_NUMBER_INIT(e, num) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_LITERAL_NUMBER), \
        .value = (num), \
    }

static inline void bae_literal_number_init(struct bae_literal_number *n, int num)
{
    *n = (struct bae_literal_number)BAE_LITERAL_NUMBER_INIT(*n, num);
}

static inline struct bae_literal_number *create_bae_literal_number(struct bcc_ast *ast, int val)
{
    struct bae_literal_number *lit_num = bcc_ast_entry_alloc(ast, struct bae_literal_number, NULL);
    bae_literal_number_init(lit_num, val);
    return lit_num;
}

#endif
