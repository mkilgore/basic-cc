#ifndef INCLUDE_AST_NODES_BINARY_OP_H
#define INCLUDE_AST_NODES_BINARY_OP_H

#include <stdlib.h>
#include "ast/ast.h"

enum bcc_ast_binary_op {
    BCC_AST_BINARY_OP_PLUS,
    BCC_AST_BINARY_OP_MINUS,
    BCC_AST_BINARY_OP_MULT,
    BCC_AST_BINARY_OP_DIV,
};

struct bae_binary_op {
    struct bcc_ast_entry ent;
    enum bcc_ast_binary_op op;
    struct bcc_ast_entry *left, *right;
};

#define BAE_BINARY_OP(e, op_type) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_BINARY_OP), \
        .op = (op_type), \
    }

static inline void bae_binary_op_init(struct bae_binary_op *b, enum bcc_ast_binary_op t)
{
    *b = (struct bae_binary_op)BAE_BINARY_OP(*b, t);
}

static inline struct bae_binary_op *create_bae_binary_op(enum bcc_ast_binary_op op)
{
    struct bae_binary_op *bin_op = malloc(sizeof(*bin_op));
    bae_binary_op_init(bin_op, op);
    return bin_op;
}


#endif
