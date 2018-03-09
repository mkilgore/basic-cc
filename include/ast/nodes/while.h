#ifndef INCLUDE_AST_NODES_WHILE_H
#define INCLUDE_AST_NODES_WHILE_H

#include <stdlib.h>
#include "ast/ast.h"

struct bae_while {
    struct bcc_ast_entry ent;
    struct bcc_ast_entry *condition;
    struct bcc_ast_entry *block;
};

#define BAE_WHILE_INIT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_WHILE), \
    }

static inline void bae_while_init(struct bae_while *i)
{
    *i = (struct bae_while)BAE_WHILE_INIT(*i);
}

static inline struct bae_while *create_bae_while(void)
{
    struct bae_while *w = malloc(sizeof(*w));
    bae_while_init(w);
    return w;
}

#endif
