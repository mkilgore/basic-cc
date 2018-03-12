
#include <stdlib.h>
#include "ast.h"

void bae_if_clear(struct bcc_ast_entry *fentry)
{
    struct bae_if *f = container_of(fentry, struct bae_if, ent);

    bcc_ast_entry_clear(f->if_expression);
    free(f->if_expression);

    bcc_ast_entry_clear(f->block);
    free(f->block);

    if (f->else_block) {
        bcc_ast_entry_clear(f->else_block);
        free(f->else_block);
    }
}

