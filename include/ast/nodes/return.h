#ifndef INCLUDE_AST_NODES_RETURN_H
#define INCLUDE_AST_NODES_RETURN_H

#include <stdlib.h>
#include "ast/ast.h"

struct bae_return {
    struct bcc_ast_entry ent;
    struct bcc_ast_entry *ret_value;
};

#define BAE_RETURN_INIT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_RETURN), \
    }

static inline void bae_return_init(struct bae_return *b)
{
    *b = (struct bae_return)BAE_RETURN_INIT(*b);
}

static inline struct bae_return *create_bae_return(struct bcc_ast *ast)
{
    struct bae_return *ret = bcc_ast_entry_alloc(ast, struct bae_return, NULL);
    bae_return_init(ret);
    return ret;
}

#endif
