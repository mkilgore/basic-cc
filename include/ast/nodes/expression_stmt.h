#ifndef INCLUDE_AST_NODES_EXPRESSION_STMT_H
#define INCLUDE_AST_NODES_EXPRESSION_STMT_H

#include <stdlib.h>
#include "ast/ast.h"

struct bae_expression_stmt {
    struct bcc_ast_entry ent;
    struct bcc_ast_entry *expression;
};

#define BAE_EXPRESSION_STMT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_EXPRESSION_STMT), \
    }

static inline void bae_expression_stmt_init(struct bae_expression_stmt *s)
{
    *s = (struct bae_expression_stmt)BAE_EXPRESSION_STMT(*s);
}

static inline struct bae_expression_stmt *create_bae_expression_stmt(struct bcc_ast *ast)
{
    struct bae_expression_stmt *stmt = bcc_ast_entry_alloc(ast, struct bae_expression_stmt, NULL);
    bae_expression_stmt_init(stmt);
    return stmt;
}

#endif
