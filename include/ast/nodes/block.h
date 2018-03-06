#ifndef INCLUDE_AST_NODES_BLOCK_H
#define INCLUDE_AST_NODES_BLOCK_H

#include "ast/ast.h"

struct bae_block {
    struct bcc_ast_entry ent;
    struct bae_block *outer_block;
    int ent_count;
    list_head_t entry_list;
    list_head_t variable_list;
};

#define BAE_BLOCK(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_BLOCK), \
        .entry_list = LIST_HEAD_INIT((e).entry_list), \
        .variable_list = LIST_HEAD_INIT((e).variable_list), \
    }

static inline void bae_block_init(struct bae_block *b)
{
    *b = (struct bae_block)BAE_BLOCK(*b);
}

#endif
