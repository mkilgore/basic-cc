
#include <stdbool.h>

#include "ast.h"
#include "bits.h"
#include "specifier_utils.h"

bool specifier_is_invalid(flags_t specifiers, int new_specifier)
{
    flags_t flags;

    if (flag_test(&specifiers, new_specifier))
        return true;

    switch (new_specifier) {
    case BCC_AST_TYPE_SPECIFIER_CHAR:
        flags = F(BCC_AST_TYPE_SPECIFIER_SIGNED) | F(BCC_AST_TYPE_SPECIFIER_UNSIGNED);
        return (specifiers & ~flags) != 0;

    case BCC_AST_TYPE_SPECIFIER_VOID:
        return specifiers != 0;

    case BCC_AST_TYPE_SPECIFIER_SHORT:
        flags = F(BCC_AST_TYPE_SPECIFIER_INT) | F(BCC_AST_TYPE_SPECIFIER_SIGNED) | F(BCC_AST_TYPE_SPECIFIER_UNSIGNED);
        return (specifiers & ~flags) != 0;

    case BCC_AST_TYPE_SPECIFIER_LONG:
        flags = F(BCC_AST_TYPE_SPECIFIER_INT) | F(BCC_AST_TYPE_SPECIFIER_SIGNED) | F(BCC_AST_TYPE_SPECIFIER_UNSIGNED);
        return (specifiers & ~flags) != 0;

    case BCC_AST_TYPE_SPECIFIER_INT:
        flags = F(BCC_AST_TYPE_SPECIFIER_SIGNED) | F(BCC_AST_TYPE_SPECIFIER_UNSIGNED) | F(BCC_AST_TYPE_SPECIFIER_LONG) | F(BCC_AST_TYPE_SPECIFIER_SHORT);
        return (specifiers & ~flags) != 0;

    case BCC_AST_TYPE_SPECIFIER_SIGNED:
    case BCC_AST_TYPE_SPECIFIER_UNSIGNED:
        flags = F(BCC_AST_TYPE_SPECIFIER_SIGNED) | F(BCC_AST_TYPE_SPECIFIER_UNSIGNED);
        return (specifiers & flags) != 0;
    }

    return false;
}

void specifier_add_to_type(struct bcc_ast_type *type, int specifier)
{
    switch (specifier) {
    case BCC_AST_TYPE_SPECIFIER_VOID:
        type->prim = BCC_AST_PRIM_VOID;
        break;

    case BCC_AST_TYPE_SPECIFIER_INT:
        if (type->prim == BCC_AST_PRIM_MAX) {
            type->size = bcc_ast_type_primitives[BCC_AST_PRIM_INT].size;
            type->prim = BCC_AST_PRIM_INT;
        }
        break;

    case BCC_AST_TYPE_SPECIFIER_LONG:
        type->size = bcc_ast_type_primitives[BCC_AST_PRIM_LONG].size;
        type->prim = BCC_AST_PRIM_LONG;
        break;

    case BCC_AST_TYPE_SPECIFIER_SHORT:
        type->size = bcc_ast_type_primitives[BCC_AST_PRIM_SHORT].size;
        type->prim = BCC_AST_PRIM_SHORT;
        break;

    case BCC_AST_TYPE_SPECIFIER_CHAR:
        type->size = bcc_ast_type_primitives[BCC_AST_PRIM_CHAR].size;
        type->prim = BCC_AST_PRIM_CHAR;
        break;

    case BCC_AST_TYPE_SPECIFIER_UNSIGNED:
        type->is_unsigned = 1;
        if (type->prim == BCC_AST_PRIM_MAX) {
            type->size = bcc_ast_type_primitives[BCC_AST_PRIM_INT].size;
            type->prim = BCC_AST_PRIM_INT;
        }
        break;

    case BCC_AST_TYPE_SPECIFIER_SIGNED:
        if (type->prim == BCC_AST_PRIM_MAX) {
            type->size = bcc_ast_type_primitives[BCC_AST_PRIM_INT].size;
            type->prim = BCC_AST_PRIM_INT;
        }
        break;
    }
}

