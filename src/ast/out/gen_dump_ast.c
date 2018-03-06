
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ast.h"

struct print_state {
    int indent;
    FILE *out;
};

static void bcc_ast_entry_print(struct print_state *, struct bcc_ast_entry *ent);

static void print_state_out(struct print_state *state, const char *str, ...)
{
    int i;
    for (i = 0; i < state->indent; i++)
        fprintf(state->out, "    ");

    va_list lst;
    va_start(lst, str);
    vfprintf(state->out, str, lst);
    va_end(lst);
}

static void print_node_literal_number(struct print_state *state, struct bcc_ast_entry *ent)
{
    struct bae_literal_number *lit_num = container_of(ent, struct bae_literal_number, ent);
    print_state_out(state, "BCC_AST_NODE_LITERAL_NUMBER: %d\n", lit_num->value);
}

static void print_node_var_load(struct print_state *state, struct bcc_ast_entry *ent)
{
    struct bae_var_load *var = container_of(ent, struct bae_var_load, ent);
    print_state_out(state, "BCC_AST_NODE_VAR_LOAD: %s\n", var->var->name);
}

static void print_node_var_store(struct print_state *state, struct bcc_ast_entry *ent)
{
    struct bae_var_load *var = container_of(ent, struct bae_var_load, ent);
    print_state_out(state, "BCC_AST_NODE_VAR_STORE: %s\n", var->var->name);
}

static void print_node_binary_op(struct print_state *state, struct bcc_ast_entry *ent)
{
    struct bae_binary_op *bin_op = container_of(ent, struct bae_binary_op, ent);
    const char *bin_op_str = "";

    switch (bin_op->op) {
    case BCC_AST_BINARY_OP_PLUS:
        bin_op_str = " + ";
        break;

    case BCC_AST_BINARY_OP_MINUS:
        bin_op_str = " - ";
        break;

    case BCC_AST_BINARY_OP_MULT:
        bin_op_str = " * ";
        break;

    case BCC_AST_BINARY_OP_DIV:
        bin_op_str = " / ";
        break;
    }


    print_state_out(state, "BCC_AST_NODE_BINARY_OP: %s\n", bin_op_str);

    print_state_out(state, "LEFT_EXP:\n");
    bcc_ast_entry_print(state, bin_op->left);

    print_state_out(state, "RIGHT_EXP:\n");
    bcc_ast_entry_print(state, bin_op->right);
}

static void print_node_expression_stmt(struct print_state *state, struct bcc_ast_entry *ent)
{
    struct bae_expression_stmt *stmt = container_of(ent, struct bae_expression_stmt, ent);

    print_state_out(state, "BCC_AST_NODE_EXPRESSION_STMT\n");
    bcc_ast_entry_print(state, stmt->expression);
}

static void print_node_if(struct print_state *state, struct bcc_ast_entry *ent)
{
    struct bae_if *if_op = container_of(ent, struct bae_if, ent);

    print_state_out(state, "BCC_AST_NODE_IF\n");

    print_state_out(state, "IF_EXPRESSION:\n");
    bcc_ast_entry_print(state, if_op->if_expression);

    print_state_out(state, "IF_BLOCK:\n");
    bcc_ast_entry_print(state, if_op->block);

    if (if_op->else_block) {
        print_state_out(state, "IF_ELSE_BLOCK:\n");
        bcc_ast_entry_print(state, if_op->else_block);
    }
}

static void print_node_function(struct print_state *state, struct bcc_ast_entry *ent)
{
    struct bae_function *func = container_of(ent, struct bae_function, ent);
    struct bcc_ast_variable *var;

    print_state_out(state, "BCC_AST_NODE_FUNCTION: %s\n", func->name);

    list_foreach_entry(&func->param_list, var, block_entry) {
        print_state_out(state, "PARAM: %s\n", var->name);
    }

    if (func->block)
        bcc_ast_entry_print(state, func->block);
}

static void print_node_func_call(struct print_state *state, struct bcc_ast_entry *ent)
{
    struct bae_func_call *call = container_of(ent, struct bae_func_call, ent);
    struct bcc_ast_entry *entry;

    print_state_out(state, "BCC_AST_NODE_FUNC_CALL: %s\n", call->func->name);

    list_foreach_entry(&call->param_list, entry, entry) {
        print_state_out(state, "PARAM:\n");
        bcc_ast_entry_print(state, entry);
    }
}

static void print_node_block(struct print_state *state, struct bcc_ast_entry *ent)
{
    struct bae_block *block = container_of(ent, struct bae_block, ent);
    struct bcc_ast_entry *entry;
    struct bcc_ast_variable *var;

    print_state_out(state, "BCC_AST_NODE_BLOCK\n");

    list_foreach_entry(&block->variable_list, var, block_entry)
        print_state_out(state, "VAR: %s;\n", var->name);

    list_foreach_entry(&block->entry_list, entry, entry)
        bcc_ast_entry_print(state, entry);
}

static void print_node_assign(struct print_state *state, struct bcc_ast_entry *ent)
{
    struct bae_assign *assign = container_of(ent, struct bae_assign, ent);

    print_state_out(state, "BCC_AST_NODE_ASSIGN\n");

    print_state_out(state, "LVALUE:\n");
    bcc_ast_entry_print(state, assign->lvalue);

    print_state_out(state, "RVALUE:\n");
    bcc_ast_entry_print(state, assign->rvalue);
}

static void print_node_return(struct print_state *state, struct bcc_ast_entry *ent)
{
    struct bae_return *ret = container_of(ent, struct bae_return, ent);
    print_state_out(state, "BCC_AST_NODE_RETURN\n");
    bcc_ast_entry_print(state, ret->ret_value);
}

static void print_node_while(struct print_state *state, struct bcc_ast_entry *ent)
{
    struct bae_while *w = container_of(ent, struct bae_while, ent);
    print_state_out(state, "BCC_AST_NODE_WHILE:\n");
    print_state_out(state, "CONDITION:\n");
    bcc_ast_entry_print(state, w->condition);
    print_state_out(state, "BLOCK:\n");
    bcc_ast_entry_print(state, w->block);
}

static void (*print_node_table[BCC_AST_NODE_MAX])(struct print_state *, struct bcc_ast_entry *) = {
    [BCC_AST_NODE_LITERAL_NUMBER] = print_node_literal_number,
    [BCC_AST_NODE_VAR_LOAD] = print_node_var_load,
    [BCC_AST_NODE_VAR_STORE] = print_node_var_store,
    [BCC_AST_NODE_BINARY_OP] = print_node_binary_op,
    [BCC_AST_NODE_EXPRESSION_STMT] = print_node_expression_stmt,
    [BCC_AST_NODE_IF] = print_node_if,
    [BCC_AST_NODE_FUNCTION] = print_node_function,
    [BCC_AST_NODE_FUNC_CALL] = print_node_func_call,
    [BCC_AST_NODE_BLOCK] = print_node_block,
    [BCC_AST_NODE_ASSIGN] = print_node_assign,
    [BCC_AST_NODE_RETURN] = print_node_return,
    [BCC_AST_NODE_WHILE] = print_node_while,
};

static void bcc_ast_entry_print(struct print_state *state, struct bcc_ast_entry *ent)
{
    void (*print_func)(struct print_state *, struct bcc_ast_entry *);

    print_func = print_node_table[ent->type];

    state->indent++;
    (print_func) (state, ent);
    state->indent--;
}

void gen_dump_ast(struct bcc_ast *ast, FILE *out)
{
    struct print_state state;
    struct bae_function *func;

    memset(&state, 0, sizeof(state));
    state.out = out;

    list_foreach_entry(&ast->function_list, func, function_entry) {
        print_state_out(&state, "FUNCTION: %s\n", func->name);
        bcc_ast_entry_print(&state, &func->ent);
    }
}

