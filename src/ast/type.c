
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "container_of.h"

#include "ast.h"

#define DEF_PRIM(t, s) \
    [t] = { \
        .node_type = BCC_AST_TYPE_PRIM, \
        .prim = (t), \
        .size = (s), \
    }

struct bcc_ast_type bcc_ast_type_primitives[BCC_AST_PRIM_MAX] = {
    DEF_PRIM(BCC_AST_PRIM_LONG, 4),
    DEF_PRIM(BCC_AST_PRIM_INT, 4),
    DEF_PRIM(BCC_AST_PRIM_SHORT, 2),
    DEF_PRIM(BCC_AST_PRIM_CHAR, 1),
    DEF_PRIM(BCC_AST_PRIM_VOID, 0),
};

struct bcc_ast_type bcc_ast_type_int_zero = {
    .node_type = BCC_AST_TYPE_PRIM,
    .prim = BCC_AST_PRIM_INT,
    .size = 4,
    .is_zero = 1,
};

struct bcc_ast_type bcc_ast_type_char_ptr = {
    .node_type = BCC_AST_TYPE_POINTER,
    .size = BCC_AST_TYPE_POINTER_SIZE,
    .inner = bcc_ast_type_primitives + BCC_AST_PRIM_CHAR,
};

struct bcc_ast_type *create_bcc_ast_type(struct bcc_ast *ast)
{
    struct bcc_ast_type *type = object_pool_get(&ast->type_object_pool);
    bcc_ast_type_init(type);
    return type;
}

/* Compares two types and decies if the rvalue can be assigned to the lvalue without casting */
bool bcc_ast_type_lvalue_identical_to_rvalue(struct bcc_ast_type *lvalue, struct bcc_ast_type *rvalue)
{
    int count = 0;

    for (; lvalue && rvalue; lvalue = lvalue->inner, rvalue = rvalue->inner, count++) {
        if (lvalue->node_type != rvalue->node_type)
            return false;

        /* 
         * Const gets interesting. 'top-level' const can be ignored (assignments are copies, so it doesn't matter if the rvalue is const)
         * Also, 'second-level' const can be ignored if the 'const' is on the lvalue. IE:
         *
         *     const char *s = (char *)l
         *
         * Const past the second level cannot be dropped however, so this is never legal:
         *
         *     const char **s = (char **)l
         */

        if (count >= 2) {
            if (lvalue->qualifier_flags != rvalue->qualifier_flags)
                return false;
        } else if (count == 0) {
            /* First level - ignore const completely */
            flags_t lvalue_flags = lvalue->qualifier_flags & ~F(BCC_AST_TYPE_QUALIFIER_CONST);
            flags_t rvalue_flags = rvalue->qualifier_flags & ~F(BCC_AST_TYPE_QUALIFIER_CONST);
            if (lvalue_flags != rvalue_flags)
                return false;
        } else {
            /* Second level - ignore const only if lvalue is const */
            if (flag_test(&lvalue->qualifier_flags, BCC_AST_TYPE_QUALIFIER_CONST)) {
                flags_t lvalue_flags = lvalue->qualifier_flags & ~F(BCC_AST_TYPE_QUALIFIER_CONST);
                flags_t rvalue_flags = rvalue->qualifier_flags & ~F(BCC_AST_TYPE_QUALIFIER_CONST);
                if (lvalue_flags != rvalue_flags)
                    return false;
            } else {
                if (lvalue->qualifier_flags != rvalue->qualifier_flags)
                    return false;
            }
        }

        if (lvalue->node_type == BCC_AST_TYPE_PRIM) {
            if (lvalue->prim != rvalue->prim)
                return false;

            if (lvalue->is_unsigned != rvalue->is_unsigned)
                return false;
        }
    }

    if (lvalue || rvalue)
        return false;

    return true;
}

bool bcc_ast_type_are_identical(struct bcc_ast_type *first, struct bcc_ast_type *second)
{
    for (; first && second; first = first->inner, second = second->inner) {
        if (first->node_type != second->node_type)
            return false;

        if (first->node_type == BCC_AST_TYPE_PRIM) {
            if (first->prim != second->prim)
                return false;

            if (first->is_unsigned != second->is_unsigned)
                return false;
        }
    }

    if (first || second)
        return false;

    return true;
}

bool bcc_ast_type_is_void_pointer(struct bcc_ast_type *t)
{
    if (t->node_type != BCC_AST_TYPE_POINTER)
        return false;

    t = t->inner;

    if (t->node_type != BCC_AST_TYPE_PRIM || t->prim != BCC_AST_PRIM_VOID)
        return false;

    return true;
}

bool bcc_ast_type_implicit_cast_exists(struct bcc_ast_type *current, struct bcc_ast_type *target)
{
    /* Pointers can be implicit cast to and from void * */
    if (bcc_ast_type_is_void_pointer(current) && bcc_ast_type_is_pointer(target))
        return true;

    if (bcc_ast_type_is_void_pointer(target) && bcc_ast_type_is_pointer(current))
        return true;

    /* Zero is implicitly treated as the null pointer constant and can be assigned to any pointer */
    if (bcc_ast_type_is_pointer(target))
        if (bcc_ast_type_is_integer(current) && current->is_zero)
            return true;

    /* Casts between integers are always valid */
    if (bcc_ast_type_is_integer(current) && bcc_ast_type_is_integer(target))
        return true;

    /* Else, an implicit cast does not exist */
    return false;
}

#if 0
/* Returns the resulting target type */
struct bcc_ast_type *bcc_ast_type_implicit_cast_exists(struct bcc_ast_type *first, struct bcc_ast_type *second)
{
    /* Void * casts */
    if (is_void_pointer(first) && second->node_type == BCC_AST_TYPE_POINTER)
        return second;

    if (is_void_pointer(second) && first->node_type == BCC_AST_TYPE_POINTER)
        return first;

    /* If we don't have a void *, then there are no other implicit conversions if either type is a pointer */
    if (first->node_type == BCC_AST_TYPE_POINTER || second->node_type == BCC_AST_TYPE_POINTER)
        return NULL;

    /* Int casts go here */
    return NULL;
}
#endif

bool bcc_ast_type_is_integer(struct bcc_ast_type *type)
{
    if (type->node_type != BCC_AST_TYPE_PRIM)
        return false;

    switch (type->prim) {
    case BCC_AST_PRIM_CHAR:
    case BCC_AST_PRIM_SHORT:
    case BCC_AST_PRIM_LONG:
    case BCC_AST_PRIM_INT:
        return true;

    default:
        return false;
    }
}

struct bcc_ast_type *bcc_ast_type_integer_promotion(struct bcc_ast_type *first, struct bcc_ast_type *second)
{
    return 0;
}

int bcc_ast_type_are_compatible(struct bcc_ast_type *first, struct bcc_ast_type *second)
{
    return 0;
}

char *bcc_ast_type_get_name(struct bcc_ast_type *type)
{
    char *str = malloc(255);
    size_t len = 0;

    for (; type; type = type->inner) {
        if (type->node_type == BCC_AST_TYPE_POINTER) {
            len += snprintf(str + len, 255 - len, "pointer to ");
        } else if (type->node_type == BCC_AST_TYPE_PRIM) {
            switch (type->prim) {
            case BCC_AST_PRIM_INT:
                len += snprintf(str + len, 255 - len, "int");
                break;

            case BCC_AST_PRIM_CHAR:
                len += snprintf(str + len, 255 - len, "char");
                break;

            case BCC_AST_PRIM_SHORT:
                len += snprintf(str + len, 255 - len, "short");
                break;

            case BCC_AST_PRIM_LONG:
                len += snprintf(str + len, 255 - len, "long");
                break;

            case BCC_AST_PRIM_VOID:
                len += snprintf(str + len, 255 - len, "void");
                break;


            default:
                len += snprintf(str + len, 255 - len, "unknown");
                break;
            }
        }
    }

    return str;
}

