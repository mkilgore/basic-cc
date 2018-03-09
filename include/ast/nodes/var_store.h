#ifndef INCLUDE_AST_NODES_VAR_STORE_H
#define INCLUDE_AST_NODES_VAR_STORE_H

#include <stdlib.h>
#include "ast/ast.h"

struct bae_var_store {
    struct bcc_ast_entry ent;
    struct bcc_ast_variable *var;
};

#define BAE_VAR_STORE_INIT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_VAR_STORE), \
    }

static inline void bae_var_store_init(struct bae_var_store *n)
{
    *n = (struct bae_var_store)BAE_VAR_STORE_INIT(*n);
}

static inline struct bae_var_store *create_bae_var_store(void)
{
    struct bae_var_store *var = malloc(sizeof(*var));
    bae_var_store_init(var);
    return var;
}

#endif
