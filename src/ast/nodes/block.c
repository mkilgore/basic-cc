
#include <stdlib.h>
#include "ast.h"

void bae_block_clear(struct bcc_ast_entry *eblock)
{
    struct bae_block *block = container_of(eblock, struct bae_block, ent);
    struct bcc_ast_variable *var;

    list_foreach_take_entry(&block->variable_list, var, block_entry) {
        bcc_ast_variable_clear(var);
        free(var);
    }
}

