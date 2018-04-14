#ifndef SRC_AST_IN_LALR_FUNC_PARAM_CHECK_H
#define SRC_AST_IN_LALR_FUNC_PARAM_CHECK_H

#include <stdbool.h>
#include "ast.h"

bool func_params_check_and_cast(struct bcc_ast *, struct bcc_ast_type *func_type, list_head_t *params, const char **errmsg);
//bool func_params_check_and_cast(struct bcc_ast *, struct bae_function *, list_head_t *params, const char **errmsg);
struct bcc_ast_type *func_type_from_parts(struct bcc_ast *ast, list_head_t *params, struct bcc_ast_type *ret_type, int has_ellipsis);

#endif
