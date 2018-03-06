#ifndef INCLUDE_AST_VARIABLE_H
#define INCLUDE_AST_VARIABLE_H

#include <stdint.h>

#include "ast/ast.h"
#include "list.h"

struct bcc_ast_variable {
    char *name;

    list_node_t func_entry;
    list_node_t block_entry;

    /* Note the offset is from the current stack frame
     * Offset is always positive, even though stack grows downward */
    uint32_t loffset;
};

#define BCC_AST_VARIABLE_INIT(e) \
    { \
        .block_entry = LIST_NODE_INIT((e).block_entry), \
        .func_entry = LIST_NODE_INIT((e).func_entry), \
    }

static inline void bcc_ast_variable_init(struct bcc_ast_variable *var)
{
    *var = (struct bcc_ast_variable)BCC_AST_VARIABLE_INIT(*var);
}

#endif