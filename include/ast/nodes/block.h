#ifndef INCLUDE_AST_NODES_BLOCK_H
#define INCLUDE_AST_NODES_BLOCK_H

#include <stdlib.h>
#include "ast/ast.h"

struct bae_block {
    struct bcc_ast_entry ent;
    struct bae_block *outer_block;
    int ent_count;
    list_head_t entry_list;
    list_head_t variable_list;
};

void bae_block_clear(struct bcc_ast_entry *);

#define BAE_BLOCK(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_BLOCK, bae_block_clear), \
        .entry_list = LIST_HEAD_INIT((e).entry_list), \
        .variable_list = LIST_HEAD_INIT((e).variable_list), \
    }

static inline void bae_block_init(struct bae_block *b)
{
    *b = (struct bae_block)BAE_BLOCK(*b);
}

static inline struct bae_block *create_bae_block(void)
{
    struct bae_block *block = malloc(sizeof(*block));
    bae_block_init(block);
    return block;
}

static inline void bae_block_add_entry(struct bae_block *block, struct bcc_ast_entry *ent)
{
    block->ent_count++;
    list_add_tail(&block->entry_list, &ent->entry);
}

#endif
