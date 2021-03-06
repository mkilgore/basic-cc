#ifndef INCLUDE_AST_VARIABLE_H
#define INCLUDE_AST_VARIABLE_H

#include <stdint.h>
#include <stdlib.h>

#include "ast.h"
#include "type.h"
#include "list.h"

struct bcc_ast_variable {
    char *name;
    struct bcc_ast_type *type;

    /* Used for list of variables within a function */
    list_node_t func_entry;

    /* Used for list of variables within a block */
    list_node_t block_entry;

    /* Used if this is a temporary variable used by the compiler
     * Note they are all the same size, a pointer length. */
    list_node_t func_temp_entry;

    /* Used for temporary list in the parser */
    list_node_t temp_entry;

    /* Note the offset is from the current stack frame */
    int32_t loffset;
};

#define BCC_AST_VARIABLE_INIT(e) \
    { \
        .block_entry = LIST_NODE_INIT((e).block_entry), \
        .func_entry = LIST_NODE_INIT((e).func_entry), \
        .temp_entry = LIST_NODE_INIT((e).temp_entry), \
    }

static inline void bcc_ast_variable_init(struct bcc_ast_variable *var)
{
    *var = (struct bcc_ast_variable)BCC_AST_VARIABLE_INIT(*var);
}

static inline void bcc_ast_variable_clear(struct bcc_ast_variable *var)
{
    free(var->name);
}

static inline struct bcc_ast_variable *create_bcc_ast_variable(void)
{
    struct bcc_ast_variable *var = malloc(sizeof(*var));
    bcc_ast_variable_init(var);
    return var;
}

#endif
