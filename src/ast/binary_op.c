
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "container_of.h"

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
};

const char *binary_op_string(enum bcc_ast_binary_op bin_op)
{
    return binary_op_str_table[bin_op];
}

