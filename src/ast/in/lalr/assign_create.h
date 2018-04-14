#ifndef SRC_AST_IN_LALR_ASSIGN_CREATE_H
#define SRC_AST_IN_LALR_ASSIGN_CREATE_H

#include "parser.h"
#include "ast.h"

struct bcc_ast_entry *create_assignment(struct bcc_ast *ast, struct bcc_parser_state *state, struct bcc_ast_entry *lvalue, struct bcc_ast_entry *rvalue, enum bcc_ast_binary_op bin_op);

#endif
