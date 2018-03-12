
#include "ast.h"

void bae_binary_op_clear(struct bcc_ast_entry *entry)
{
    struct bae_binary_op *bin_op = container_of(entry, struct bae_binary_op, ent);

    bcc_ast_entry_clear(bin_op->left);
    free(bin_op->left);

    bcc_ast_entry_clear(bin_op->right);
    free(bin_op->right);
}

