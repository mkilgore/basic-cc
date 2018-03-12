
#include <stdlib.h>
#include "ast.h"

void bae_expression_stmt_clear(struct bcc_ast_entry *entry)
{
    struct bae_expression_stmt *stmt = container_of(entry, struct bae_expression_stmt, ent);

    bcc_ast_entry_clear(stmt->expression);
    free(stmt->expression);
}

