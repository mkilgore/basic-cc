#ifndef INCLUDE_AST_NODES_IF_H
#define INCLUDE_AST_NODES_IF_H

#include <stdlib.h>
#include "ast/ast.h"

struct bae_if {
    struct bcc_ast_entry ent;
    struct bcc_ast_entry *if_expression;
    struct bcc_ast_entry *block;
    struct bcc_ast_entry *else_block;
};

#define BAE_IF_INIT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_IF), \
    }

static inline void bae_if_init(struct bae_if *i)
{
    *i = (struct bae_if)BAE_IF_INIT(*i);
}

static inline struct bae_if *create_bae_if(void)
{
    struct bae_if *i = malloc(sizeof(*i));
    bae_if_init(i);
    return i;
}

#endif
