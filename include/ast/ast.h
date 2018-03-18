#ifndef INCLUDE_AST_AST_H
#define INCLUDE_AST_AST_H

#include <stdio.h>
#include <stdbool.h>
#include "list.h"
#include "type.h"
#include "object_pool.h"

enum bcc_ast_node_type {
    BCC_AST_NODE_LITERAL_NUMBER,
    BCC_AST_NODE_LITERAL_STRING,
    BCC_AST_NODE_UNARY_OP,
    BCC_AST_NODE_BINARY_OP,
    BCC_AST_NODE_EXPRESSION_STMT,
    BCC_AST_NODE_IF,
    BCC_AST_NODE_FUNCTION,
    BCC_AST_NODE_BLOCK,
    BCC_AST_NODE_RETURN,
    BCC_AST_NODE_VAR,
    BCC_AST_NODE_VAR_LOAD,
    BCC_AST_NODE_VAR_STORE,
    BCC_AST_NODE_ASSIGN,
    BCC_AST_NODE_FUNC_CALL,
    BCC_AST_NODE_WHILE,
    BCC_AST_NODE_CAST,
    BCC_AST_NODE_MAX
};

struct bcc_ast_entry {
    enum bcc_ast_node_type type;
    struct bcc_ast_type *node_type;
    list_node_t entry;

    void (*clear) (struct bcc_ast_entry *);
};

#define BCC_AST_ENTRY_INIT(e, t, cle) \
    { \
        .type = (t), \
        .entry = LIST_NODE_INIT((e).entry), \
        .clear = (cle), \
    }

static inline size_t bae_size(struct bcc_ast_entry *ent)
{
    return ent->node_type->size;
}

struct bcc_ast {
    int function_count;
    list_head_t function_list;

    int next_string_id;
    list_head_t literal_string_list;

    int type_count;
    struct bcc_ast_type *types;

    /* This object pool holds all of the struct bcc_ast_type objects.
     * This is done because a single `struct bcc_ast_type` object may be
     * referenced by other `struct bcc_ast_type` objects, making it very hard
     * to keep track of references and free them at the end.
     *
     * It is more efficent to simply allocate them from a single allocator and
     * then free them all at once. */
    struct object_pool type_object_pool;
};

/* Allocate two pages worth of memory at a time for the type objects */
#define BCC_AST_TYPE_POOL_SIZE (4096 * 2 / sizeof(struct bcc_ast_type))

#define BCC_AST_INIT(ast) \
    { \
        .function_list = LIST_HEAD_INIT((ast).function_list), \
        .literal_string_list = LIST_HEAD_INIT((ast).literal_string_list), \
        .type_object_pool = OBJECT_POOL_INIT(sizeof(struct bcc_ast_type), BCC_AST_TYPE_POOL_SIZE), \
    }

static inline void bcc_ast_init(struct bcc_ast *ast)
{
    *ast = (struct bcc_ast)BCC_AST_INIT(*ast);
}

void bcc_ast_clear(struct bcc_ast *ast);

static inline void bcc_ast_entry_clear(struct bcc_ast_entry *ent)
{
    if (ent)
        (ent->clear) (ent);
}

enum bcc_ast_out_format {
    BCC_AST_OUT_ASM_X86,
    BCC_AST_OUT_TEXT,
    BCC_AST_OUT_DUMP_AST,
};

struct bae_block;
struct bae_function;
struct bae_literal_string;
struct bcc_ast_variable;

void bcc_ast_out(struct bcc_ast *ast, FILE *out, enum bcc_ast_out_format);
int bcc_ast_parse(struct bcc_ast *ast, FILE *in);
struct bcc_ast_variable *bcc_ast_find_variable(struct bcc_ast *ast, struct bae_block *scope, const char *name);
struct bae_function *bcc_ast_find_function(struct bcc_ast *ast, const char *name);
void bcc_ast_add_function(struct bcc_ast *ast, struct bae_function *func);
void bcc_ast_add_literal_string(struct bcc_ast *ast, struct bae_literal_string *lit_str);

bool bcc_ast_entry_is_lvalue(struct bcc_ast_entry *);

#endif
