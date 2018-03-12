
#include <stdlib.h>
#include "ast.h"

void bae_func_call_clear(struct bcc_ast_entry *fentry)
{
    struct bae_func_call *call = container_of(fentry, struct bae_func_call, ent);
    struct bcc_ast_entry *ent;

    list_foreach_take_entry(&call->param_list, ent, entry) {
        bcc_ast_entry_clear(ent);
        free(ent);
    }
}

