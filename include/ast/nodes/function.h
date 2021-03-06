#ifndef INCLUDE_AST_NODES_FUNCTION_H
#define INCLUDE_AST_NODES_FUNCTION_H

#include <stdlib.h>
#include <stdint.h>
#include "ast/ast.h"

struct bae_function {
    struct bcc_ast_entry ent;
    list_node_t function_entry;
    list_head_t local_variable_list;
    list_head_t param_list;

    list_head_t compiler_temp_var_list;
    int compiler_temps_total;
    int compiler_temps_in_use;

    uint8_t has_ellipsis;

    char *name;
    struct bcc_ast_type *ret_type;
    struct bcc_ast_entry *block;
};

void bae_function_clear(struct bcc_ast_entry *);

#define BAE_FUNCTION_INIT(e, nam) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_FUNCTION), \
        .name = (nam), \
        .function_entry = LIST_NODE_INIT((e).function_entry), \
        .local_variable_list = LIST_HEAD_INIT((e).local_variable_list), \
        .param_list = LIST_HEAD_INIT((e).param_list), \
        .compiler_temp_var_list = LIST_HEAD_INIT((e).compiler_temp_var_list), \
    }

static inline void bae_function_init(struct bae_function *f, char *name)
{
    *f = (struct bae_function)BAE_FUNCTION_INIT(*f, name);
}

static inline struct bae_function *create_bae_function(struct bcc_ast *ast, char *name)
{
    struct bae_function *func = bcc_ast_entry_alloc(ast, struct bae_function, bae_function_clear);
    bae_function_init(func, name);
    return func;
}

struct bcc_ast_variable *bae_function_get_temp_var(struct bae_function *func);
void bae_function_put_temp_var(struct bae_function *func, struct bcc_ast_variable *var);

#endif
