#ifndef INCLUDE_AST_NODES_RETURN_H
#define INCLUDE_AST_NODES_RETURN_H

#include <stdlib.h>
#include "ast/ast.h"

struct bae_return {
    struct bcc_ast_entry ent;
    struct bcc_ast_entry *ret_value;
};

void bae_return_clear(struct bcc_ast_entry *);

#define BAE_RETURN_INIT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_RETURN, bae_return_clear), \
    }

static inline void bae_return_init(struct bae_return *b)
{
    *b = (struct bae_return)BAE_RETURN_INIT(*b);
}

static inline struct bae_return *create_bae_return(void)
{
    struct bae_return *ret = malloc(sizeof(*ret));
    bae_return_init(ret);
    return ret;
}

#endif
