
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "container_of.h"

#include "ast.h"

#define DEF_PRIM(t) \
    { \
        .node_type = BCC_AST_TYPE_PRIM, \
        .prim = (t), \
    }

struct bcc_ast_type bcc_ast_type_primitives[BCC_AST_PRIM_MAX] = {
    DEF_PRIM(BCC_AST_PRIM_INT),
    DEF_PRIM(BCC_AST_PRIM_CHAR),
};

struct bcc_ast_type bcc_ast_type_char_ptr = {
    .node_type = BCC_AST_TYPE_POINTER,
    .inner = bcc_ast_type_primitives + BCC_AST_PRIM_CHAR,
};

struct bcc_ast_type *create_bcc_ast_type(struct bcc_ast *ast)
{
    struct bcc_ast_type *type = object_pool_get(&ast->type_object_pool);
    bcc_ast_type_init(type);object_pool_get(&ast->type_object_pool);
    return type;
}

bool bcc_ast_type_are_identical(struct bcc_ast_type *first, struct bcc_ast_type *second)
{
    for (; first && second; first = first->inner, second = second->inner) {
        if (first->node_type != second->node_type)
            return false;

        if (first->node_type == BCC_AST_TYPE_PRIM)
            if (first->prim != second->prim)
                return false;
    }

    if (first || second)
        return false;

    return true;
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

            default:
                len += snprintf(str + len, 255 - len, "unknown");
                break;
            }
        }
    }

    return str;
}

