
#include "ast.h"

void bae_assign_clear(struct bcc_ast_entry *entry)
{
    struct bae_assign *assign = container_of(entry, struct bae_assign, ent);

    bcc_ast_entry_clear(assign->lvalue);
    free(assign->lvalue);

    bcc_ast_entry_clear(assign->rvalue);
    free(assign->rvalue);
}

