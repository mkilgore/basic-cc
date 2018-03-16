#ifndef INCLUDE_AST_NODES_CAST_H
#define INCLUDE_AST_NODES_CAST_H

#include "ast/ast.h"

struct bae_cast {
    struct bcc_ast_entry ent;
    struct bcc_ast_entry *expr;
    struct bcc_ast_type *target;
};

void bae_cast_clear(struct bcc_ast_entry *);

#define BAE_CAST_INIT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_CAST, bae_cast_clear), \
    }

static inline void bae_cast_init(struct bae_cast *cast)
{
    *cast = (struct bae_cast)BAE_CAST_INIT(*cast);
}

static inline struct bae_cast *create_bae_cast(void)
{
    struct bae_cast *cast = malloc(sizeof(*cast));
    bae_cast_init(cast);
    return cast;
}

#endif
