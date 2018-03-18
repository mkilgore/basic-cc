#ifndef INCLUDE_AST_NODES_ASSIGN_H
#define INCLUDE_AST_NODES_ASSIGN_H

#include <stdlib.h>
#include "ast/ast.h"

struct bae_assign {
    struct bcc_ast_entry ent;
    struct bcc_ast_entry *optional_expr;
    struct bcc_ast_entry *rvalue;
    struct bcc_ast_entry *lvalue;
};

void bae_assign_clear(struct bcc_ast_entry *);

#define BAE_ASSIGN_INIT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_ASSIGN, bae_assign_clear), \
    }

static inline void bae_assign_init(struct bae_assign *n)
{
    *n = (struct bae_assign)BAE_ASSIGN_INIT(*n);
}

static inline struct bae_assign *create_bae_assign(void)
{
    struct bae_assign *assign = malloc(sizeof(*assign));
    bae_assign_init(assign);
    return assign;
}

#endif
