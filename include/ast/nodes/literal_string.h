#ifndef INCLUDE_AST_NODES_LITERAL_STRING_H
#define INCLUDE_AST_NODES_LITERAL_STRING_H

#include <stdlib.h>
#include "ast/ast.h"

struct bae_literal_string {
    struct bcc_ast_entry ent;
    char *str;

    int string_id;

    list_node_t literal_string_entry;
};

void bae_literal_string_clear(struct bcc_ast_entry *);

#define BAE_LITERAL_STRING_INIT(e) \
    { \
        .ent = BCC_AST_ENTRY_INIT((e).ent, BCC_AST_NODE_LITERAL_STRING, bae_literal_string_clear), \
        .literal_string_entry = LIST_NODE_INIT((e).literal_string_entry), \
    }

static inline void bae_literal_string_init(struct bae_literal_string *n)
{
    *n = (struct bae_literal_string)BAE_LITERAL_STRING_INIT(*n);
}

static inline struct bae_literal_string *create_bae_literal_string(void)
{
    struct bae_literal_string *s = malloc(sizeof(*s));
    bae_literal_string_init(s);
    return s;
}

#endif
