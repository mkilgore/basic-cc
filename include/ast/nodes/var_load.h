#ifndef INCLUDE_AST_NODES_VAR_LOAD_H
#define INCLUDE_AST_NODES_VAR_LOAD_H

#include <stdlib.h>
#include "ast/ast.h"

struct bae_var_load {
    struct bcc_ast_entry ent;
    struct bcc_ast_variable *var;
};

#define BAE_VAR_LOAD_INIT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_VAR_LOAD), \
    }

static inline void bae_var_load_init(struct bae_var_load *n)
{
    *n = (struct bae_var_load)BAE_VAR_LOAD_INIT(*n);
}

static inline struct bae_var_load *create_bae_var_load(void)
{
    struct bae_var_load *var = malloc(sizeof(*var));
    bae_var_load_init(var);
    return var;
}

#endif
