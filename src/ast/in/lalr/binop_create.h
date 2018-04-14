#ifndef SRC_AST_IN_LALR_BINOP_CREATE_H
#define SRC_AST_IN_LALR_BINOP_CREATE_H

#include "ast.h"

struct bcc_ast_entry *create_bin_from_components(struct bcc_ast *ast, enum bcc_ast_binary_op op, struct bcc_ast_entry *left, struct bcc_ast_entry *right);

struct bcc_ast_entry *create_ptr_bin_from_components(struct bcc_ast *ast, enum bcc_ast_binary_op op, struct bcc_ast_entry *left, struct bcc_ast_entry *right);

struct bcc_ast_entry *create_bin_ptr_op(struct bcc_ast *ast, enum bcc_ast_binary_op op, struct bcc_ast_entry *ptr, struct bcc_ast_entry *val, int ptr_is_first);

#endif
