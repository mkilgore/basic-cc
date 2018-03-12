
#include <stdlib.h>
#include "ast.h"

void bae_return_clear(struct bcc_ast_entry *rentry)
{
    struct bae_return *ret = container_of(rentry, struct bae_return, ent);
    bcc_ast_entry_clear(ret->ret_value);
    free(ret->ret_value);
}

