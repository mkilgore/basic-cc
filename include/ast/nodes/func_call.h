#ifndef INCLUDE_AST_NODES_FUNC_CALL_H
#define INCLUDE_AST_NODES_FUNC_CALL_H

#include <stdlib.h>
#include "ast/ast.h"

struct bae_func_call {
    struct bcc_ast_entry ent;
    list_head_t param_list;
    struct bae_function *func;
};

#define BAE_FUNC_CALL_INIT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_FUNC_CALL), \
        .param_list = LIST_HEAD_INIT((e).param_list), \
    }

static inline void bae_func_call_init(struct bae_func_call *b)
{
    *b = (struct bae_func_call)BAE_FUNC_CALL_INIT(*b);
}

static inline struct bae_func_call *create_bae_func_call(struct bcc_ast *ast)
{
    struct bae_func_call *call = bcc_ast_entry_alloc(ast, struct bae_func_call, NULL);
    bae_func_call_init(call);
    return call;
}

#endif
