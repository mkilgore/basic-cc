
#include <stdio.h>
#include <string.h>

#include "ast.h"

struct gen_state {
    int next_label;
    FILE *out;
};

static void gen_bcc_ast_entry(struct gen_state *, struct bcc_ast_entry *ent, FILE *);

static void gen_node_literal_number(struct gen_state *state, struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_literal_number *lit_num = container_of(ent, struct bae_literal_number, ent);
    fprintf(out, "    movl $%d, %%eax\n", lit_num->value);
}

static void gen_node_var_load(struct gen_state *state, struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_var_load *load = container_of(ent, struct bae_var_load, ent);
    fprintf(out, "    movl -%d(%%ebp), %%eax\n", load->var->loffset);
}

static void gen_node_var_store(struct gen_state *state, struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_var_store *store = container_of(ent, struct bae_var_store, ent);
    fprintf(out, "    movl %%eax, -%d(%%ebp)\n", store->var->loffset);
}

static void gen_node_binary_op(struct gen_state *state, struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_binary_op *bin_op = container_of(ent, struct bae_binary_op, ent);

    gen_bcc_ast_entry(state, bin_op->left, out);
    fprintf(out, "    pushl %%eax\n");
    gen_bcc_ast_entry(state, bin_op->right, out);
    fprintf(out, "    popl %%ebx\n");

    switch (bin_op->op) {
    case BCC_AST_BINARY_OP_PLUS:
        fprintf(out, "    addl %%ebx, %%eax\n");
        break;

    case BCC_AST_BINARY_OP_MINUS:
        fprintf(out, "    subl %%eax, %%ebx\n");
        fprintf(out, "    movl %%ebx, %%eax\n");
        break;

    case BCC_AST_BINARY_OP_MULT:
        fprintf(out, "    imull %%ebx, %%eax\n");
        break;

    case BCC_AST_BINARY_OP_DIV:
        fprintf(out, "    xchg %%eax, %%ebx\n");
        fprintf(out, "    cltd\n");
        fprintf(out, "    idivl %%ebx\n");
        break;
    }
}

static void gen_node_expression_stmt(struct gen_state *state, struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_expression_stmt *stmt = container_of(ent, struct bae_expression_stmt, ent);
    gen_bcc_ast_entry(state, stmt->expression, out);
}

static void gen_node_if(struct gen_state *state, struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_if *if_op = container_of(ent, struct bae_if, ent);
    int if_count = state->next_label++;

    gen_bcc_ast_entry(state, if_op->if_expression, out);
    fprintf(out, "    cmpl $0, %%eax\n");
    fprintf(out, "    je .L_ELSE_CLAUSE%d\n", if_count);

    gen_bcc_ast_entry(state, if_op->block, out);
    fprintf(out, "    jmp .L_END_IF%d\n", if_count);
    fprintf(out, ".L_ELSE_CLAUSE%d:\n", if_count);
    if (if_op->else_block)
        gen_bcc_ast_entry(state, if_op->else_block, out);
    fprintf(out, ".L_END_IF%d:\n", if_count);
}

static void gen_node_func_call(struct gen_state *state, struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_func_call *call = container_of(ent, struct bae_func_call, ent);
    struct bcc_ast_entry *entry;
    int param_count = 0;

    list_foreach_entry(&call->param_list, entry, entry) {
        gen_bcc_ast_entry(state, entry, out);
        fprintf(out, "    pushl %%eax\n");
        param_count++;
    }

    fprintf(out, "    call %s\n", call->func->name);
    fprintf(out, "    addl $%d, %%esp\n", param_count * 4);
}

static void gen_node_block(struct gen_state *state, struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_block *block = container_of(ent, struct bae_block, ent);
    struct bcc_ast_entry *entry;

    list_foreach_entry(&block->entry_list, entry, entry)
        gen_bcc_ast_entry(state, entry, out);
}

static void gen_node_assign(struct gen_state *state, struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_assign *assign = container_of(ent, struct bae_assign, ent);

    gen_bcc_ast_entry(state, assign->rvalue, out);
    gen_bcc_ast_entry(state, assign->lvalue, out);
}

static void gen_node_return(struct gen_state *state, struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_return *ret = container_of(ent, struct bae_return, ent);
    gen_bcc_ast_entry(state, ret->ret_value, out);

    /* Return value already in eax */
    fprintf(out, "    leave\n");
    fprintf(out, "    ret\n");
}

static void gen_node_while(struct gen_state *state, struct bcc_ast_entry *ent, FILE *out)
{
    struct bae_while *w = container_of(ent, struct bae_while, ent);
    int label = state->next_label++;

    fprintf(out, ".L_WHILE_CONDITION%d:\n", label);

    gen_bcc_ast_entry(state, w->condition, out);
    fprintf(out, "    cmpl $0, %%eax\n");
    fprintf(out, "    je .L_WHILE_END%d\n", label);

    gen_bcc_ast_entry(state, w->block, out);
    fprintf(out, "    jmp .L_WHILE_CONDITION%d\n", label);

    fprintf(out, ".L_WHILE_END%d:\n", label);
}

void (*gen_entry_table[BCC_AST_NODE_MAX]) (struct gen_state *state, struct bcc_ast_entry *ent, FILE *) = {
    [BCC_AST_NODE_LITERAL_NUMBER] = gen_node_literal_number,
    [BCC_AST_NODE_VAR_LOAD] = gen_node_var_load,
    [BCC_AST_NODE_VAR_STORE] = gen_node_var_store,
    [BCC_AST_NODE_BINARY_OP] = gen_node_binary_op,
    [BCC_AST_NODE_EXPRESSION_STMT] = gen_node_expression_stmt,
    [BCC_AST_NODE_IF] = gen_node_if,
    [BCC_AST_NODE_FUNC_CALL] = gen_node_func_call,
    [BCC_AST_NODE_BLOCK] = gen_node_block,
    [BCC_AST_NODE_ASSIGN] = gen_node_assign,
    [BCC_AST_NODE_RETURN] = gen_node_return,
    [BCC_AST_NODE_WHILE] = gen_node_while,
};

static void gen_bcc_ast_entry(struct gen_state *state, struct bcc_ast_entry *ent, FILE *out)
{
    void (*gen_func) (struct gen_state *, struct bcc_ast_entry *, FILE *);

    gen_func = gen_entry_table[ent->type];

    if (gen_func)
        (gen_func) (state, ent, out);
}

static void gen_bcc_ast_function(struct gen_state *state, struct bae_function *func, FILE *out)
{
    struct bcc_ast_variable *var;
    int local_var_count = 0;

    if (func->block) {
        fprintf(out, ".global %s\n", func->name);
        fprintf(out, "%s:\n",  func->name);
        fprintf(out, "    pushl %%ebp\n");
        fprintf(out, "    movl %%esp, %%ebp\n");

        list_foreach_entry(&func->local_variable_list, var, func_entry) {
            local_var_count++;
            var->loffset = local_var_count * 4;
        }

        fprintf(out, "    subl $%d, %%esp\n", local_var_count * 4);

        gen_bcc_ast_entry(state, func->block, out);
    } else {
        fprintf(out, ".extern %s\n", func->name);
    }
}

void gen_asm_x86(struct bcc_ast *ast, FILE *out)
{
    struct gen_state state;
    struct bae_function *func;

    memset(&state, 0, sizeof(state));

    state.out = out;

    list_foreach_entry(&ast->function_list, func, function_entry)
        gen_bcc_ast_function(&state, func, out);
}

