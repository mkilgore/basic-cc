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

#define BAE_ASSIGN_INIT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_ASSIGN), \
    }

static inline void bae_assign_init(struct bae_assign *n)
{
    *n = (struct bae_assign)BAE_ASSIGN_INIT(*n);
}

static inline struct bae_assign *create_bae_assign(struct bcc_ast *ast)
{
    struct bae_assign *assign = bcc_ast_entry_alloc(ast, struct bae_assign, NULL);
    bae_assign_init(assign);
    return assign;
}

#endif
