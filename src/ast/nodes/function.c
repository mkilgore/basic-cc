
#include <stdlib.h>
#include "ast.h"

void bae_function_clear(struct bcc_ast_entry *fentry)
{
    struct bae_function *func = container_of(fentry, struct bae_function, ent);
    struct bcc_ast_variable *var;

    list_foreach_take_entry(&func->param_list, var, block_entry) {
        bcc_ast_variable_clear(var);
        free(var);
    }

    free(func->name);
}

struct bcc_ast_variable *bae_function_get_temp_var(struct bae_function *func)
{
    if (func->compiler_temps_in_use < func->compiler_temps_total) {
        struct bcc_ast_variable *var;
        int count = 0;
        list_foreach_entry(&func->compiler_temp_var_list, var, func_temp_entry) {
            if (count == func->compiler_temps_in_use)
                break;
            count++;
        }

        func->compiler_temps_in_use++;
        return var;
    }

    struct bcc_ast_variable *var = create_bcc_ast_variable();
    var->type = bcc_ast_type_primitives + BCC_AST_PRIM_LONG;

    list_add_tail(&func->local_variable_list, &var->func_entry);
    list_add_tail(&func->compiler_temp_var_list, &var->func_temp_entry);

    func->compiler_temps_in_use++;
    func->compiler_temps_total++;

    return var;
}

void bae_function_put_temp_var(struct bae_function *func, struct bcc_ast_variable *temp_var)
{
    struct bcc_ast_variable *var;
    int count = 0;

    return ;

    func->compiler_temps_in_use--;

    list_foreach_entry(&func->compiler_temp_var_list, var, func_temp_entry) {
        if (count == func->compiler_temps_in_use)
            break;
        count++;
    }

    if (var != temp_var)
        printf("PARSER ERROR: Temp variables not released in the correct order\n");
}

