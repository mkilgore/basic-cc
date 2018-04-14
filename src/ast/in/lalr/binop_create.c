
#include <stdlib.h>

#include "ast.h"
#include "binop_create.h"

struct bcc_ast_entry *create_bin_ptr_op(struct bcc_ast *ast, enum bcc_ast_binary_op op, struct bcc_ast_entry *ptr, struct bcc_ast_entry *val, int ptr_is_first)
{
    size_t ptr_size = ptr->node_type->inner->size;

    if (val->node_type->size < ptr_size) {
        struct bae_cast *cast = create_bae_cast(ast);
        cast->expr = val;
        cast->target = bcc_ast_type_primitives + BCC_AST_PRIM_LONG;
        cast->ent.node_type = cast->target;

        val = &cast->ent;
    }

    struct bae_binary_op *bin_op = create_bae_binary_op(ast, op);
    bin_op->operand_size = ptr_size;
    bin_op->ent.node_type = ptr->node_type;

    if (ptr_is_first) {
        bin_op->left = ptr;
        bin_op->right = val;
    } else {
        bin_op->left = val;
        bin_op->right = ptr;
    }

    return &bin_op->ent;
}

struct bcc_ast_entry *create_ptr_bin_from_components(struct bcc_ast *ast, enum bcc_ast_binary_op op, struct bcc_ast_entry *left, struct bcc_ast_entry *right)
{
    struct bae_binary_op *bin_op;

    /* Verify that one side is a pointer, and one side is an integer */
    switch (op) {
    case BCC_AST_BINARY_OP_PLUS:
        if (bcc_ast_type_is_pointer(left->node_type) && bcc_ast_type_is_integer(right->node_type)) {
            return create_bin_ptr_op(ast, BCC_AST_BINARY_OP_ADDR_ADD_INDEX, left, right, 1);
        } else if (bcc_ast_type_is_pointer(right->node_type) && bcc_ast_type_is_integer(left->node_type)) {
            return create_bin_ptr_op(ast, BCC_AST_BINARY_OP_ADDR_ADD_INDEX, left, right, 0);
        } else {
            return NULL;
        }
        break;

    case BCC_AST_BINARY_OP_MINUS:
        if (bcc_ast_type_is_pointer(left->node_type) && bcc_ast_type_is_integer(right->node_type)) {
            return create_bin_ptr_op(ast, BCC_AST_BINARY_OP_ADDR_SUB_INDEX, left, right, 1);
        } else if (bcc_ast_type_is_pointer(right->node_type) && bcc_ast_type_is_integer(left->node_type)) {
            return create_bin_ptr_op(ast, BCC_AST_BINARY_OP_ADDR_SUB_INDEX, left, right, 0);
        } else if (bcc_ast_type_is_pointer(right->node_type) && bcc_ast_type_is_pointer(left->node_type)) {
            struct bae_binary_op *bin_op = create_bae_binary_op(ast, BCC_AST_BINARY_OP_ADDR_SUB);
            bin_op->operand_size = bae_size(right);
            bin_op->left = left;
            bin_op->right = right;
            bin_op->ent.node_type = bcc_ast_type_primitives + BCC_AST_PRIM_INT;
            return &bin_op->ent;
        } else {
            return NULL;
        }
        break;

    case BCC_AST_BINARY_OP_GREATER_THAN:
    case BCC_AST_BINARY_OP_GREATER_THAN_EQUAL:
    case BCC_AST_BINARY_OP_LESS_THAN:
    case BCC_AST_BINARY_OP_LESS_THAN_EQUAL:
    case BCC_AST_BINARY_OP_NOT_EQUAL:
    case BCC_AST_BINARY_OP_DOUBLEEQUAL:
        bin_op = create_bae_binary_op(ast, op);

        bin_op->left = left;
        bin_op->right = right;
        bin_op->ent.node_type = bcc_ast_type_primitives + BCC_AST_PRIM_INT;
        return &bin_op->ent;

    default:
        return NULL;
    }
}

struct bcc_ast_entry *create_bin_from_components(struct bcc_ast *ast, enum bcc_ast_binary_op op, struct bcc_ast_entry *left, struct bcc_ast_entry *right)
{
    /* This does necessary things like integer promotion, and array to pointer, and function to function pointer */
    left = bcc_ast_entry_conv_with_promotion(ast, left);
    right = bcc_ast_entry_conv_with_promotion(ast, right);

    if (bcc_ast_type_is_pointer(left->node_type) || bcc_ast_type_is_pointer(right->node_type))
        return create_ptr_bin_from_components(ast, op, left, right);

    if (bcc_ast_type_is_integer(left->node_type) && bcc_ast_type_is_integer(right->node_type)) {
        size_t size_left = bae_size(left);
        size_t size_right = bae_size(right);

        /* FIXME: Follow signed and unsigned casting rules here */

        if (size_left < size_right) {
            struct bae_cast *cast = create_bae_cast(ast);
            cast->expr = left;
            cast->target = right->node_type;
            cast->ent.node_type = cast->target;

            left = &cast->ent;
        } else if (size_right < size_left) {
            struct bae_cast *cast = create_bae_cast(ast);
            cast->expr = right;
            cast->target = left->node_type;
            cast->ent.node_type = cast->target;

            right = &cast->ent;
        }
    }

    struct bae_binary_op *bin_op = create_bae_binary_op(ast, op);

    bin_op->left = left;
    bin_op->right = right;
    bin_op->ent.node_type = bin_op->left->node_type;
    return &bin_op->ent;
}

