#ifndef INCLUDE_AST_NODES_BINARY_OP_H
#define INCLUDE_AST_NODES_BINARY_OP_H

#include <stdlib.h>
#include "ast/ast.h"

enum bcc_ast_binary_op {
    BCC_AST_BINARY_OP_PLUS,
    BCC_AST_BINARY_OP_MINUS,
    BCC_AST_BINARY_OP_MULT,
    BCC_AST_BINARY_OP_DIV,
    BCC_AST_BINARY_OP_MOD,

    BCC_AST_BINARY_OP_SHIFTRIGHT,
    BCC_AST_BINARY_OP_SHIFTLEFT,

    BCC_AST_BINARY_OP_GREATER_THAN,
    BCC_AST_BINARY_OP_LESS_THAN,
    BCC_AST_BINARY_OP_GREATER_THAN_EQUAL,
    BCC_AST_BINARY_OP_LESS_THAN_EQUAL,
    BCC_AST_BINARY_OP_DOUBLEEQUAL,
    BCC_AST_BINARY_OP_NOT_EQUAL,

    BCC_AST_BINARY_OP_BITWISE_AND,
    BCC_AST_BINARY_OP_BITWISE_OR,
    BCC_AST_BINARY_OP_BITWISE_XOR,

    BCC_AST_BINARY_OP_LOGICAL_OR,
    BCC_AST_BINARY_OP_LOGICAL_AND,

    BCC_AST_BINARY_OP_ADDR_ADD_INDEX, /* This is addition/subtraction of pointer with an integer */
    BCC_AST_BINARY_OP_ADDR_SUB_INDEX,

    BCC_AST_BINARY_OP_ADDR_SUB, /* This is subtraction of two pointers */

    BCC_AST_BINARY_OP_MAX
};

struct bae_binary_op {
    struct bcc_ast_entry ent;
    enum bcc_ast_binary_op op;

    struct bcc_ast_entry *left, *right;

    size_t operand_size;
};

void bae_binary_op_clear(struct bcc_ast_entry *);

#define BAE_BINARY_OP(e, op_type) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_BINARY_OP, bae_binary_op_clear), \
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

const char *binary_op_string(enum bcc_ast_binary_op bin_op);

#endif
