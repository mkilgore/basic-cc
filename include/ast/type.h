#ifndef INCLUDE_AST_TYPE_H
#define INCLUDE_AST_TYPE_H

#include <stdbool.h>
#include "bits.h"

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
    BCC_AST_TYPE_FUNCTION,
    BCC_AST_TYPE_MAX
};

/* Flags */
enum bcc_ast_type_qualifier {
    BCC_AST_TYPE_QUALIFIER_CONST,
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

    flags_t qualifier_flags;

    unsigned int is_unsigned :1;
    unsigned int is_zero :1; /* Zero is special, it can be assigned to a pointer to get NULL */
    unsigned int has_ellipsis :1;

    list_node_t param_entry;
    list_head_t param_list;

    struct bcc_ast_type *inner;
};

#define BCC_AST_TYPE(typ) \
    { \
        .param_entry = LIST_NODE_INIT((typ).param_entry), \
        .param_list = LIST_HEAD_INIT((typ).param_list), \
    }

static inline void bcc_ast_type_init(struct bcc_ast_type *type)
{
    *type = (struct bcc_ast_type)BCC_AST_TYPE(*type);
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

static inline struct bcc_ast_type *create_bcc_ast_type_function(struct bcc_ast *ast, struct bcc_ast_type *ret)
{
    struct bcc_ast_type *type = create_bcc_ast_type(ast);
    type->node_type = BCC_AST_TYPE_FUNCTION;
    type->size = 0;
    type->inner = ret;
    return type;
}

extern struct bcc_ast_type bcc_ast_type_primitives[BCC_AST_PRIM_MAX];
extern struct bcc_ast_type bcc_ast_type_char_ptr;
extern struct bcc_ast_type bcc_ast_type_int_zero;

/* Does a shallow copy of a type - the returned type object is a new copy, but the 'inner' points to the same object as the original.
 * For BCC_AST_TYPE_FUNCTION, the type objects in the param list are also copied. */
struct bcc_ast_type *bcc_ast_type_clone(struct bcc_ast *ast, struct bcc_ast_type *original);

bool bcc_ast_type_lvalue_identical_to_rvalue(struct bcc_ast_type *lvalue, struct bcc_ast_type *rvalue);
bool bcc_ast_type_are_identical(struct bcc_ast_type *first, struct bcc_ast_type *second);
int bcc_ast_type_are_compatible(struct bcc_ast_type *first, struct bcc_ast_type *second);
char *bcc_ast_type_get_name(struct bcc_ast_type *type);

bool bcc_ast_type_is_void_pointer(struct bcc_ast_type *);
bool bcc_ast_type_is_integer(struct bcc_ast_type *);

static inline bool bcc_ast_type_is_primitive(struct bcc_ast_type *type)
{
    return type->node_type == BCC_AST_TYPE_PRIM;
}

static inline bool bcc_ast_type_is_primitive_specific(struct bcc_ast_type *type, enum bcc_ast_primitive_type prim_type)
{
    return bcc_ast_type_is_primitive(type) && type->prim == prim_type;
}

static inline bool bcc_ast_type_is_pointer(struct bcc_ast_type *type)
{
    return type->node_type == BCC_AST_TYPE_POINTER;
}

static inline bool bcc_ast_type_is_const(struct bcc_ast_type *type)
{
    return flag_test(&type->qualifier_flags, BCC_AST_TYPE_QUALIFIER_CONST);
}

static inline bool bcc_ast_type_is_unsigned(struct bcc_ast_type *type)
{
    return type->is_unsigned;
}

//struct bcc_ast_type *bcc_ast_type_integer_promotion(struct bcc_ast_type *first, struct bcc_ast_type *second);

/* Checks for an implicit cast between 'first' and 'second'.  */
bool bcc_ast_type_implicit_cast_exists(struct bcc_ast_type *current, struct bcc_ast_type *target);

#endif
