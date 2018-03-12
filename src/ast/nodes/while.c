
#include <stdlib.h>
#include "ast.h"

void bae_while_clear(struct bcc_ast_entry *wentry)
{
    struct bae_while *whil = container_of(wentry, struct bae_while, ent);

    bcc_ast_entry_clear(whil->condition);
    free(whil->condition);

    bcc_ast_entry_clear(whil->block);
    free(whil->block);
}

