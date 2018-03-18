#ifndef INCLUDE_AST_NODES_VAR_H
#define INCLUDE_AST_NODES_VAR_H

#include <stdlib.h>
#include "ast/ast.h"

struct bae_var {
    struct bcc_ast_entry ent;
    struct bcc_ast_variable *var;
};

void bae_var_clear(struct bcc_ast_entry *);

#define BAE_VAR_INIT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_VAR, bae_var_clear), \
    }

static inline void bae_var_init(struct bae_var *n)
{
    *n = (struct bae_var)BAE_VAR_INIT(*n);
}

static inline struct bae_var *create_bae_var(void)
{
    struct bae_var *var = malloc(sizeof(*var));
    bae_var_init(var);
    return var;
}


#endif
