#ifndef INCLUDE_AST_NODES_CAST_H
#define INCLUDE_AST_NODES_CAST_H

#include "ast/ast.h"

struct bae_cast {
    struct bcc_ast_entry ent;
    struct bcc_ast_entry *expr;
    struct bcc_ast_type *target;
};

#define BAE_CAST_INIT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_CAST), \
    }

static inline void bae_cast_init(struct bae_cast *cast)
{
    *cast = (struct bae_cast)BAE_CAST_INIT(*cast);
}

static inline struct bae_cast *create_bae_cast(struct bcc_ast *ast)
{
    struct bae_cast *cast = bcc_ast_entry_alloc(ast, struct bae_cast, NULL);
    bae_cast_init(cast);
    return cast;
}

#endif
