#ifndef INCLUDE_AST_NODES_UNARY_OP_H
#define INCLUDE_AST_NODES_UNARY_OP_H

#include <stdlib.h>
#include "ast/ast.h"

enum bcc_ast_unary_op {
    BCC_AST_UNARY_OP_PLUSPLUS,
    BCC_AST_UNARY_OP_ADDR_OF,
    BCC_AST_UNARY_OP_DEREF,
};

struct bae_unary_op {
    struct bcc_ast_entry ent;
    enum bcc_ast_binary_op op;
    struct bcc_ast_entry *expr;
};

void bae_unary_op_clear(struct bcc_ast_entry *);

#define BAE_UNARY_OP(e, op_type) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_UNARY_OP, bae_unary_op_clear), \
        .op = (op_type), \
    }

static inline void bae_unary_op_init(struct bae_unary_op *b, enum bcc_ast_unary_op t)
{
    *b = (struct bae_unary_op)BAE_UNARY_OP(*b, t);
}

static inline struct bae_unary_op *create_bae_unary_op(enum bcc_ast_unary_op op)
{
    struct bae_unary_op *unary_op = malloc(sizeof(*unary_op));
    bae_unary_op_init(unary_op, op);
    return unary_op;
}

#endif