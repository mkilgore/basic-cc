
#include <stdlib.h>
#include "ast.h"

void bae_cast_clear(struct bcc_ast_entry *ent)
{
    struct bae_cast *cast = container_of(ent, struct bae_cast, ent);

    bcc_ast_entry_clear(cast->expr);
    free(cast->expr);
}
