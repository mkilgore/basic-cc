
#include <stdlib.h>
#include "ast.h"

void bae_unary_op_clear(struct bcc_ast_entry *uentry)
{
    struct bae_unary_op *unary = container_of(uentry, struct bae_unary_op, ent);
    bcc_ast_entry_clear(unary->expr);
    free(unary->expr);
}

