#ifndef SRC_AST_IN_LALR_SPECIFIER_UTILS_H
#define SRC_AST_IN_LALR_SPECIFIER_UTILS_H

#include "ast.h"

enum bcc_ast_type_specifier {
    BCC_AST_TYPE_SPECIFIER_INT,
    BCC_AST_TYPE_SPECIFIER_LONG,
    BCC_AST_TYPE_SPECIFIER_UNSIGNED,
    BCC_AST_TYPE_SPECIFIER_SIGNED,
    BCC_AST_TYPE_SPECIFIER_SHORT,
    BCC_AST_TYPE_SPECIFIER_CHAR,
    BCC_AST_TYPE_SPECIFIER_VOID,
};

bool specifier_is_invalid(flags_t specifiers, int new_specifier);
void specifier_add_to_type(struct bcc_ast_type *type, int specifier);

static inline enum bcc_ast_primitive_type specifier_to_prim(enum bcc_ast_type_specifier spec)
{
    switch (spec) {
    case BCC_AST_TYPE_SPECIFIER_INT:
        return BCC_AST_PRIM_INT;

    case BCC_AST_TYPE_SPECIFIER_CHAR:
        return BCC_AST_PRIM_CHAR;

    case BCC_AST_TYPE_SPECIFIER_SHORT:
        return BCC_AST_PRIM_SHORT;

    case BCC_AST_TYPE_SPECIFIER_LONG:
        return BCC_AST_PRIM_LONG;

    case BCC_AST_TYPE_SPECIFIER_VOID:
        return BCC_AST_PRIM_VOID;

    default:
        return BCC_AST_PRIM_MAX;
    }
}

#endif
