
#include <stdbool.h>

#include "ast.h"
#include "func_param_check.h"

bool func_params_check_and_cast(struct bcc_ast *ast, struct bcc_ast_type *func_type, list_head_t *params, const char **errmsg)
{
    if (!params) {
        if (!list_empty(&func_type->param_list)) {
            *errmsg = "Not enough arguments supplied to function";
            return false;
        }
    } else {
        struct bcc_ast_type *param;
        struct bcc_ast_entry *arg;

        /*
         * Type checking and casting for arguments
         * This looks horrible, but we're really just looping through two lists at a time.
         */
        int i = 0;
        for (param = list_first_entry(&func_type->param_list, struct bcc_ast_type, param_entry),
             arg   = list_first_entry(params, struct bcc_ast_entry, entry);
             !list_ptr_is_head(&func_type->param_list, &param->param_entry) && !list_ptr_is_head(params, &arg->entry);
             param = list_next_entry(param, param_entry),
             arg   = list_next_entry(arg, entry), i++) {

            if (bcc_ast_type_lvalue_identical_to_rvalue(param, arg->node_type))
                continue;

            if (bcc_ast_type_implicit_cast_exists(arg->node_type, param)) {
                struct bae_cast *cast = create_bae_cast(ast);
                cast->expr = arg;
                cast->target = param;
                cast->ent.node_type = cast->target;

                list_replace(&cast->ent.entry, &arg->entry);

                arg = &cast->ent;
                continue;
            }

            *errmsg = "Argument with invalid type";
            return false;
        }

        if (!list_ptr_is_head(&func_type->param_list, &param->param_entry)) {
            *errmsg = "Not enough arguments supplied to function";
            return false;
        }

        if (!list_ptr_is_head(params, &arg->entry) && !func_type->has_ellipsis) {
            *errmsg = "Too many arguments supplied to function";
            return false;
        }

        /* Handle ellipsis arguments. In general there is no type-checking,
         * but there are some casting rules to follow */
        for (;
            !list_ptr_is_head(params, &arg->entry);
            arg = list_next_entry(arg, entry)) {

            if (bcc_ast_type_is_integer(arg->node_type)) {
                struct bae_cast *cast = create_bae_cast(ast);
                cast->expr = arg;
                cast->target = bcc_ast_type_primitives + BCC_AST_PRIM_INT;
                cast->ent.node_type = cast->target;

                list_replace(&cast->ent.entry, &arg->entry);

                arg = &cast->ent;
                continue;
            }
        }
    }

    return true;
}

struct bcc_ast_type *func_type_from_parts(struct bcc_ast *ast, list_head_t *params, struct bcc_ast_type *ret_type, int has_ellipsis)
{
    struct bcc_ast_type *type = create_bcc_ast_type_function(ast, ret_type);
    struct bcc_ast_variable *param;

    type->has_ellipsis = has_ellipsis;

    if (!params)
        return type;

    /* Check for the special cast of `(void)` */

    for (param = list_first_entry(params, struct bcc_ast_variable, block_entry);
         !list_ptr_is_head(params, &param->block_entry);
         param = list_next_entry(param, block_entry)) {

        struct bcc_ast_type *param_type = param->type;

        param_type = bcc_ast_type_clone(ast, param_type);
        list_add_tail(&type->param_list, &param_type->param_entry);
    }

    return type;
}

