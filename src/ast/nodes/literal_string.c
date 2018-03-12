
#include <stdlib.h>
#include "ast.h"

void bae_literal_string_clear(struct bcc_ast_entry *lentry)
{
    struct bae_literal_string *lit_str = container_of(lentry, struct bae_literal_string, ent);
    free(lit_str->str);
}

