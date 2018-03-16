#ifndef INCLUDE_AST_TYPE_H
#define INCLUDE_AST_TYPE_H

#include <stdbool.h>

enum bcc_ast_primitive_type {
    BCC_AST_PRIM_VOID,
    BCC_AST_PRIM_INT,
    BCC_AST_PRIM_CHAR,
    BCC_AST_PRIM_SHORT,
    BCC_AST_PRIM_LONG,
    BCC_AST_PRIM_MAX
};

enum bcc_ast_type_node {
    BCC_AST_TYPE_PRIM,
    BCC_AST_TYPE_POINTER,
    BCC_AST_TYPE_MAX
};

/*
 * Represents type information.
 *
 * Types are either a primitive value type, or a warpper around such (Like a pointer).
 *
 * Wrappers are created by creating 'chains', where 'inner' points to the inner type being wrapped.
 */
struct bcc_ast_type {
    enum bcc_ast_type_node node_type;
    enum bcc_ast_primitive_type prim;

    size_t size;

    struct bcc_ast_type *inner;
};

#define BCC_AST_TYPE() \
    { \
    }

static inline void bcc_ast_type_init(struct bcc_ast_type *type)
{
    *type = (struct bcc_ast_type)BCC_AST_TYPE();
}

struct bcc_ast;

struct bcc_ast_type *create_bcc_ast_type(struct bcc_ast *ast);

#define BCC_AST_TYPE_POINTER_SIZE 4

static inline struct bcc_ast_type *create_bcc_ast_type_prim(struct bcc_ast *ast, enum bcc_ast_primitive_type prim)
{
    struct bcc_ast_type *type = create_bcc_ast_type(ast);
    type->node_type = BCC_AST_TYPE_PRIM;
    type->prim = prim;
    return type;
}

static inline struct bcc_ast_type *create_bcc_ast_type_pointer(struct bcc_ast *ast, struct bcc_ast_type *inner)
{
    struct bcc_ast_type *type = create_bcc_ast_type(ast);
    type->node_type = BCC_AST_TYPE_POINTER;
    type->size = BCC_AST_TYPE_POINTER_SIZE;
    type->inner = inner;
    return type;
}

extern struct bcc_ast_type bcc_ast_type_primitives[BCC_AST_PRIM_MAX];
extern struct bcc_ast_type bcc_ast_type_char_ptr;

bool bcc_ast_type_are_identical(struct bcc_ast_type *first, struct bcc_ast_type *second);
int bcc_ast_type_are_compatible(struct bcc_ast_type *first, struct bcc_ast_type *second);
char *bcc_ast_type_get_name(struct bcc_ast_type *type);

bool bcc_ast_type_is_integer(struct bcc_ast_type *);

static inline bool bcc_ast_type_is_pointer(struct bcc_ast_type *type)
{
    return type->node_type == BCC_AST_TYPE_POINTER;
}

struct bcc_ast_type *bcc_ast_type_integer_promotion(struct bcc_ast_type *first, struct bcc_ast_type *second);

/* Checks for an implicit cast between 'first' and 'second'.  */
struct bcc_ast_type *bcc_ast_type_implicit_cast_exists(struct bcc_ast_type *first, struct bcc_ast_type *second);

#endif
