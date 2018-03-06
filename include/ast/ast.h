#ifndef INCLUDE_AST_AST_H
#define INCLUDE_AST_AST_H

#include "list.h"

enum bcc_ast_node_type {
    BCC_AST_NODE_LITERAL_NUMBER,
    BCC_AST_NODE_LITERAL_STRING,
    BCC_AST_NODE_BINARY_OP,
    BCC_AST_NODE_EXPRESSION_STMT,
    BCC_AST_NODE_IF,
    BCC_AST_NODE_FUNCTION,
    BCC_AST_NODE_BLOCK,
    BCC_AST_NODE_RETURN,
    BCC_AST_NODE_VAR_LOAD,
    BCC_AST_NODE_VAR_STORE,
    BCC_AST_NODE_ASSIGN,
    BCC_AST_NODE_FUNC_CALL,
    BCC_AST_NODE_WHILE,
    BCC_AST_NODE_MAX
};

struct bcc_ast_type {

};

struct bcc_ast_entry {
    enum bcc_ast_node_type type;
    list_node_t entry;
};

#define BCC_AST_ENTRY_INIT(e, t) \
    { \
        .type = (t), \
        .entry = LIST_NODE_INIT((e).entry), \
    }

struct bcc_ast {
    int function_count;
    list_head_t function_list;

    int type_count;
    struct bcc_ast_type *types;
};

#define BCC_AST_INIT(ast) \
    { \
        .function_list = LIST_HEAD_INIT((ast).function_list), \
    }

static inline void bcc_ast_init(struct bcc_ast *ast)
{
    *ast = (struct bcc_ast)BCC_AST_INIT(*ast);
}

enum bcc_ast_out_format {
    BCC_AST_OUT_ASM_X86,
    BCC_AST_OUT_TEXT,
    BCC_AST_OUT_DUMP_AST,
};

void bcc_ast_out(struct bcc_ast *ast, FILE *out, enum bcc_ast_out_format);
int bcc_ast_parse(struct bcc_ast *ast, FILE *in);

#endif
