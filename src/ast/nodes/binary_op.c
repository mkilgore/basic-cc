
#include "ast.h"

static const char *binary_op_str_table[] = {
    [BCC_AST_BINARY_OP_PLUS]               = "+",
    [BCC_AST_BINARY_OP_MINUS]              = "-",
    [BCC_AST_BINARY_OP_MULT]               = "*",
    [BCC_AST_BINARY_OP_DIV]                = "/",
    [BCC_AST_BINARY_OP_MOD]                = "%",
    [BCC_AST_BINARY_OP_SHIFTRIGHT]         = ">>",
    [BCC_AST_BINARY_OP_SHIFTLEFT]          = "<<",
    [BCC_AST_BINARY_OP_GREATER_THAN]       = ">",
    [BCC_AST_BINARY_OP_LESS_THAN]          = "<",
    [BCC_AST_BINARY_OP_GREATER_THAN_EQUAL] = ">=",
    [BCC_AST_BINARY_OP_LESS_THAN_EQUAL]    = "<=",
    [BCC_AST_BINARY_OP_DOUBLEEQUAL]        = "==",
    [BCC_AST_BINARY_OP_NOT_EQUAL]          = "!=",
    [BCC_AST_BINARY_OP_BITWISE_AND]        = "&",
    [BCC_AST_BINARY_OP_BITWISE_OR]         = "|",
    [BCC_AST_BINARY_OP_BITWISE_XOR]        = "^",
    [BCC_AST_BINARY_OP_LOGICAL_OR]         = "||",
    [BCC_AST_BINARY_OP_LOGICAL_AND]        = "&&",
    [BCC_AST_BINARY_OP_ADDR_ADD_INDEX]     = "+",
    [BCC_AST_BINARY_OP_ADDR_SUB_INDEX]     = "-",
    [BCC_AST_BINARY_OP_ADDR_SUB]           = "-",
};

const char *binary_op_string(enum bcc_ast_binary_op bin_op)
{
    return binary_op_str_table[bin_op];
}

void bae_binary_op_clear(struct bcc_ast_entry *entry)
{
    struct bae_binary_op *bin_op = container_of(entry, struct bae_binary_op, ent);

    bcc_ast_entry_clear(bin_op->left);
    free(bin_op->left);

    bcc_ast_entry_clear(bin_op->right);
    free(bin_op->right);
}

