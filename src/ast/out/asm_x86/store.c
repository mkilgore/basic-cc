
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "ast.h"
#include "gen.h"

static void store_node_var(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_var *store = container_of(ent, struct bae_var, ent);
    gen_out(state, "    mov %r, -%d(%%ebp)\n", REG_ARG('a', store->var->type->size), store->var->loffset);
}

static void store_node_unary_op(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_unary_op *uop = container_of(ent, struct bae_unary_op, ent);

    switch (uop->op) {
    case BCC_AST_UNARY_OP_DEREF:
        /* 
         * The value to be stored is in %%eax
         * Note that we also have to leave the assigned value in %%eax at the end
         */
        gen_out(state, "    pushl %%eax\n");
        gen_bcc_ast_entry(state, uop->expr);
        gen_out(state, "    movl %%eax, %%ebx\n");
        gen_out(state, "    popl %%eax\n");
        gen_out(state, "    mov %r, (%%ebx)\n", REG_ARG('a', uop->ent.node_type->size));
        break;

    default:
        printf("ERROR: Invalid LVALUE node: UNARY OP: %d\n", uop->op);
        break;
    }
}

static void (*store_entry_table[BCC_AST_NODE_MAX]) (struct gen_state *state, struct bcc_ast_entry *ent) = {
    [BCC_AST_NODE_VAR] = store_node_var,
    [BCC_AST_NODE_UNARY_OP] = store_node_unary_op,
};

void gen_bcc_ast_store(struct gen_state *state, struct bcc_ast_entry *store)
{
    void (*func_ptr) (struct gen_state *, struct bcc_ast_entry *);

    func_ptr = store_entry_table[store->type];

    if (func_ptr)
        (func_ptr) (state, store);
    else {
        printf("ERROR: Invalid LVALUE node: NODE %d\n", store->type);
    }
}

