
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

    bcc_ast_entry_clear(func->block);
    free(func->block);
}

