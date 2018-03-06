
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"

static void bcc_ast_entry_print(struct bcc_ast_entry *ent, FILE *out);

static void print_node_literal_number(struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_literal_number *lit_num = container_of(ent, struct bae_literal_number, ent);
    fprintf(out, "%d", lit_num->value);
}

static void print_node_var_load(struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_var_load *var = container_of(ent, struct bae_var_load, ent);
    fprintf(out, "%s", var->var->name);
}

static void print_node_var_store(struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_var_store *var = container_of(ent, struct bae_var_store, ent);
    fprintf(out, "%s", var->var->name);
}

static void print_node_binary_op(struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_binary_op *bin_op = container_of(ent, struct bae_binary_op, ent);

    fprintf(out, "(");
    bcc_ast_entry_print(bin_op->left, out);

    switch (bin_op->op) {
    case BCC_AST_BINARY_OP_PLUS:
        fprintf(out, " + ");
        break;

    case BCC_AST_BINARY_OP_MINUS:
        fprintf(out, " - ");
        break;

    case BCC_AST_BINARY_OP_MULT:
        fprintf(out, " * ");
        break;

    case BCC_AST_BINARY_OP_DIV:
        fprintf(out, " / ");
        break;
    }

    bcc_ast_entry_print(bin_op->right, out);
    fprintf(out, ")");
}

static void print_node_expression_stmt(struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_expression_stmt *stmt = container_of(ent, struct bae_expression_stmt, ent);
    bcc_ast_entry_print(stmt->expression, out);
    fprintf(out, ";\n");
}

static void print_node_if(struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_if *if_op = container_of(ent, struct bae_if, ent);

    fprintf(out, "if (");
    bcc_ast_entry_print(if_op->if_expression, out);
    fprintf(out, ") {\n");
    bcc_ast_entry_print(if_op->block, out);

    if (if_op->else_block) {
        fprintf(out, "} else {\n");
        bcc_ast_entry_print(if_op->else_block, out);
    }

    fprintf(out, "}\n");
}

static void print_node_function(struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_function *func = container_of(ent, struct bae_function, ent);
    struct bcc_ast_variable *var;
    fprintf(out, "int %s (", func->name);

    list_foreach_entry(&func->param_list, var, block_entry) {
        fprintf(out, "int %s", var->name);
        if (!list_is_last(&func->param_list, &var->block_entry))
            fprintf(out, ", ");
    }

    fprintf(out, ")");

    if (func->block) {
        fprintf(out, " {\n");
        bcc_ast_entry_print(func->block, out);
        fprintf(out, "}");
    }

    fprintf(out, "\n");
}

static void print_node_block(struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_block *block = container_of(ent, struct bae_block, ent);
    struct bcc_ast_entry *entry;
    struct bcc_ast_variable *var;

    list_foreach_entry(&block->variable_list, var, block_entry)
        fprintf(out, "int %s;\n", var->name);

    list_foreach_entry(&block->entry_list, entry, entry)
        bcc_ast_entry_print(entry, out);
}

static void print_node_assign(struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_assign *assign = container_of(ent, struct bae_assign, ent);

    bcc_ast_entry_print(assign->lvalue, out);
    fprintf(out, " = ");
    bcc_ast_entry_print(assign->rvalue, out);
}

static void print_node_return(struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_return *ret = container_of(ent, struct bae_return, ent);
    fprintf(out, "return ");
    bcc_ast_entry_print(ret->ret_value, out);
    fprintf(out, ";\n");
}

static void (*print_node_table[BCC_AST_NODE_MAX])(struct bcc_ast_entry *, FILE *) = {
    [BCC_AST_NODE_LITERAL_NUMBER] = print_node_literal_number,
    [BCC_AST_NODE_VAR_LOAD] = print_node_var_load,
    [BCC_AST_NODE_VAR_STORE] = print_node_var_store,
    [BCC_AST_NODE_BINARY_OP] = print_node_binary_op,
    [BCC_AST_NODE_EXPRESSION_STMT] = print_node_expression_stmt,
    [BCC_AST_NODE_IF] = print_node_if,
    [BCC_AST_NODE_FUNCTION] = print_node_function,
    [BCC_AST_NODE_BLOCK] = print_node_block,
    [BCC_AST_NODE_ASSIGN] = print_node_assign,
    [BCC_AST_NODE_RETURN] = print_node_return,
};

static void bcc_ast_entry_print(struct bcc_ast_entry *ent, FILE *out)
{
    void (*print_func)(struct bcc_ast_entry *, FILE *);

    print_func = print_node_table[ent->type];

    (print_func) (ent, out);
}

void gen_text(struct bcc_ast *ast, FILE *out)
{
    struct bae_function *func;
    list_foreach_entry(&ast->function_list, func, function_entry) {
        bcc_ast_entry_print(&func->ent, out);
        fprintf(out, "\n");
    }
}

