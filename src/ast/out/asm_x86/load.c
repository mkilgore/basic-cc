
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "ast.h"
#include "gen.h"

static void gen_node_literal_number(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_literal_number *lit_num = container_of(ent, struct bae_literal_number, ent);
    gen_out(state, "    mov $%d, %r\n", lit_num->value, REG_ARG('a', 4));
}

static void gen_node_literal_string(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_literal_string *lit_str = container_of(ent, struct bae_literal_string, ent);

    gen_out(state, "    mov $.LS%d, %r\n", lit_str->string_id, REG_ARG('a', 4));
}

static void gen_node_var_load(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_var_load *load = container_of(ent, struct bae_var_load, ent);
    gen_out(state, "    mov -%d(%%ebp), %r\n", load->var->loffset, REG_ARG('a', load->var->type->size));
}

static void gen_node_var_store(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_var_store *store = container_of(ent, struct bae_var_store, ent);
    gen_out(state, "    mov %r, -%d(%%ebp)\n", REG_ARG('a', store->var->type->size), store->var->loffset);
}

static void gen_node_logical_op(struct gen_state *state, struct bae_binary_op *op)
{
    int label = state->next_label++;
    const char *jmp_op;
    int result;

    if (op->op == BCC_AST_BINARY_OP_LOGICAL_AND) {
        jmp_op = "    je";
        result = 1;
    } else {
        jmp_op = "    jne";
        result = 0;
    }

    /* This implements the short-circuiting logic. */
    gen_bcc_ast_entry(state, op->left);
    gen_out(state, "    cmp $0, %r\n", REG_ARG('a', bae_size(op->left)));
    gen_out(state, "    %s .L_LOGICAL_OP%d\n", jmp_op, label);

    gen_bcc_ast_entry(state, op->right);
    gen_out(state, "    cmp $0, %r\n", REG_ARG('a', bae_size(op->right)));
    gen_out(state, "    %s .L_LOGICAL_OP%d\n", jmp_op, label);
    gen_out(state, "    movl $%d, %r\n", result, REG_ARG('a', 4));

    gen_out(state, ".L_LOGICAL_OP%d:\n", label);
}

static void gen_div_reg_extend(struct gen_state *state, size_t size)
{
    switch (size) {
    case 1:
        gen_out(state, "    cbtw\n");
        break;

    case 2:
        gen_out(state, "    cwtd\n");
        break;

    case 4:
        gen_out(state, "    cltd\n");
        break;
    }
}

static void gen_node_binary_op(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_binary_op *bin_op = container_of(ent, struct bae_binary_op, ent);
    size_t left_size, right_size;
    char left_reg, right_reg;

    if (bin_op->op == BCC_AST_BINARY_OP_LOGICAL_AND || bin_op->op == BCC_AST_BINARY_OP_LOGICAL_OR) {
        gen_node_logical_op(state, bin_op);
        return ;
    }

    gen_bcc_ast_entry(state, bin_op->left);
    gen_out(state, "    pushl %%eax\n");
    gen_bcc_ast_entry(state, bin_op->right);
    gen_out(state, "    popl %%ebx\n");

    left_reg = 'b';
    left_size = bae_size(bin_op->left);

    right_reg = 'a';
    right_size = bae_size(bin_op->right);

    switch (bin_op->op) {
    case BCC_AST_BINARY_OP_PLUS:
        gen_out(state, "    add %r, %r\n", REG_ARG(left_reg, left_size), REG_ARG(right_reg, right_size));
        break;

    case BCC_AST_BINARY_OP_MINUS:
        gen_out(state, "    sub %r, %r\n", REG_ARG(right_reg, right_size), REG_ARG(left_reg, left_size));
        gen_out(state, "    mov %r, %r\n", REG_ARG(left_reg, left_size), REG_ARG(right_reg, right_size));
        break;

    case BCC_AST_BINARY_OP_MULT:
        gen_out(state, "    imul %r, %r\n", REG_ARG(left_reg, left_size), REG_ARG(right_reg, right_size));
        break;

    case BCC_AST_BINARY_OP_DIV:
        gen_out(state, "    xchg %r, %r\n", REG_ARG(right_reg, right_size), REG_ARG(left_reg, left_size));
        gen_div_reg_extend(state, left_size);
        gen_out(state, "    idiv %r\n", REG_ARG(left_reg, left_size));
        break;

    case BCC_AST_BINARY_OP_MOD:
        gen_out(state, "    xchg %r, %r\n", REG_ARG(right_reg, right_size), REG_ARG(left_reg, left_size));
        gen_div_reg_extend(state, left_size);
        gen_out(state, "    idiv %r\n", REG_ARG(left_reg, left_size));
        switch (left_size) {
        case 1:
            gen_out(state, "    mov %%ah, %%al\n");
            break;

        case 2:
            gen_out(state, "    mov %%dx, %%ax\n");
            break;

        case 4:
            gen_out(state, "    mov %%edx, %%eax\n");
            break;
        }
        break;

    case BCC_AST_BINARY_OP_SHIFTRIGHT:
        gen_out(state, "    mov %%al, %%cl\n");
        gen_out(state, "    shr %%cl, %r\n", REG_ARG(left_reg, left_size));
        gen_out(state, "    mov %r, %r\n", REG_ARG(left_reg, left_size), REG_ARG(right_reg, right_size));
        break;

    case BCC_AST_BINARY_OP_SHIFTLEFT:
        gen_out(state, "    mov %%al, %%cl\n");
        gen_out(state, "    shl %%cl, %r\n", REG_ARG(left_reg, left_size));
        gen_out(state, "    mov %r, %r\n", REG_ARG(left_reg, left_size), REG_ARG(right_reg, right_size));
        break;

    case BCC_AST_BINARY_OP_GREATER_THAN:
        gen_out(state, "    cmp %r, %r\n", REG_ARG(right_reg, right_size), REG_ARG(left_reg, left_size));
        gen_out(state, "    setg %%al\n");
        gen_conv_op(state, 'a', 1, right_size);
        break;

    case BCC_AST_BINARY_OP_LESS_THAN:
        gen_out(state, "    cmp %r, %r\n", REG_ARG(right_reg, right_size), REG_ARG(left_reg, left_size));
        gen_out(state, "    setl %%al\n");
        gen_conv_op(state, 'a', 1, right_size);
        break;

    case BCC_AST_BINARY_OP_GREATER_THAN_EQUAL:
        gen_out(state, "    cmp %r, %r\n", REG_ARG(right_reg, right_size), REG_ARG(left_reg, left_size));
        gen_out(state, "    setge %%al\n");
        gen_conv_op(state, 'a', 1, right_size);
        break;

    case BCC_AST_BINARY_OP_LESS_THAN_EQUAL:
        gen_out(state, "    cmp %r, %r\n", REG_ARG(right_reg, right_size), REG_ARG(left_reg, left_size));
        gen_out(state, "    setle %%al\n");
        gen_conv_op(state, 'a', 1, right_size);
        break;

    case BCC_AST_BINARY_OP_DOUBLEEQUAL:
        gen_out(state, "    cmp %r, %r\n", REG_ARG(right_reg, right_size), REG_ARG(left_reg, left_size));
        gen_out(state, "    sete %%al\n");
        gen_conv_op(state, 'a', 1, right_size);
        break;

    case BCC_AST_BINARY_OP_NOT_EQUAL:
        gen_out(state, "    cmp %r, %r\n", REG_ARG(right_reg, right_size), REG_ARG(left_reg, left_size));
        gen_out(state, "    setne %%al\n");
        gen_conv_op(state, 'a', 1, right_size);
        break;

    case BCC_AST_BINARY_OP_BITWISE_AND:
        gen_out(state, "    and %r, %r\n", REG_ARG(left_reg, left_size), REG_ARG(right_reg, right_size));
        break;

    case BCC_AST_BINARY_OP_BITWISE_OR:
        gen_out(state, "    or %r, %r\n", REG_ARG(left_reg, left_size), REG_ARG(right_reg, right_size));
        break;

    case BCC_AST_BINARY_OP_BITWISE_XOR:
        gen_out(state, "    xor %r, %r\n", REG_ARG(left_reg, left_size), REG_ARG(right_reg, right_size));
        break;

    case BCC_AST_BINARY_OP_ADDR_ADD_INDEX:
    case BCC_AST_BINARY_OP_ADDR_SUB_INDEX:
        if (bcc_ast_type_is_pointer(bin_op->left->node_type))
            gen_out(state, "    xchg %%eax, %%ebx\n");

        if (bin_op->op == BCC_AST_BINARY_OP_ADDR_SUB_INDEX)
            gen_out(state, "    neg %%ebx\n");

        switch (bin_op->operand_size) {
        case 1:
        case 2:
        case 4:
            gen_out(state, "    leal (%%eax,%%ebx,%d), %%eax\n", bin_op->operand_size);
            break;

        default:
            break;
        }
        break;

        break;

    case BCC_AST_BINARY_OP_ADDR_SUB:
        gen_out(state, "    subl %%ebx, %%eax\n");

        switch (bin_op->operand_size) {
        case 1:
            break;

        case 2:
            gen_out(state, "    shr %%eax, $1\n");
            break;

        case 4:
            gen_out(state, "    shr %%eax, $2\n");
            break;

        default:
            break;
        }
        break;

    case BCC_AST_BINARY_OP_LOGICAL_OR:
    case BCC_AST_BINARY_OP_LOGICAL_AND:
        /* Handled separately above */
    case BCC_AST_BINARY_OP_MAX:
        break;
    }
}

static void gen_node_unary_op(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_unary_op *uop = container_of(ent, struct bae_unary_op, ent);

    switch (uop->op) {
    case BCC_AST_UNARY_OP_PLUSPLUS:
        gen_bcc_ast_entry(state, uop->expr);
        gen_out(state, "    inc %r\n", REG_ARG('a', bae_size(uop->expr)));
        gen_bcc_ast_store(state, uop->lvalue);
        break;

    case BCC_AST_UNARY_OP_PLUSPLUS_POSTFIX:
        gen_bcc_ast_entry(state, uop->expr);
        gen_out(state, "    pushl %%eax\n");
        gen_out(state, "    inc %r\n", REG_ARG('a', bae_size(uop->expr)));
        gen_bcc_ast_store(state, uop->lvalue);
        gen_out(state, "    popl %%eax\n");
        break;

    case BCC_AST_UNARY_OP_MINUSMINUS:
        gen_bcc_ast_entry(state, uop->expr);
        gen_out(state, "    dec %r\n", REG_ARG('a', bae_size(uop->expr)));
        gen_bcc_ast_store(state, uop->lvalue);
        break;

    case BCC_AST_UNARY_OP_MINUSMINUS_POSTFIX:
        gen_bcc_ast_entry(state, uop->expr);
        gen_out(state, "    pushl %%eax\n");
        gen_out(state, "    dec %r\n", REG_ARG('a', bae_size(uop->expr)));
        gen_bcc_ast_store(state, uop->lvalue);
        gen_out(state, "    popl %%eax\n");
        break;

    case BCC_AST_UNARY_OP_PLUS:
        gen_bcc_ast_entry(state, uop->expr);
        break;

    case BCC_AST_UNARY_OP_MINUS:
        gen_bcc_ast_entry(state, uop->expr);
        gen_out(state, "    neg %r\n", REG_ARG('a', bae_size(uop->expr)));
        break;

    case BCC_AST_UNARY_OP_ADDRESS_OF:
        switch (uop->expr->type) {
        case BCC_AST_NODE_VAR: {
            struct bae_var *store = container_of(uop->expr, struct bae_var, ent);
            struct bcc_ast_variable *var = store->var;
            gen_out(state, "    lea -%d(%%esp), %r\n", var->loffset, REG_ARG('a', var->type->size));
            break;
        }

        default:
            break;
        }
        break;

    case BCC_AST_UNARY_OP_DEREF:
        gen_bcc_ast_entry(state, uop->expr);
        gen_out(state, "    mov (%%eax), %r\n", REG_ARG('a', uop->ent.node_type->size));
        break;

    case BCC_AST_UNARY_OP_NOT:
        gen_bcc_ast_entry(state, uop->expr);
        gen_out(state, "    cmp $0, %r\n", REG_ARG('a', bae_size(uop->expr)));
        gen_out(state, "    setne %%eax\n");
        break;

    case BCC_AST_UNARY_OP_BITWISE_NOT:
        gen_bcc_ast_entry(state, uop->expr);
        gen_out(state, "    not %r\n", REG_ARG('a', bae_size(uop->expr)));
        break;

    case BCC_AST_UNARY_OP_MAX:
        break;
    }
}

static void gen_node_expression_stmt(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_expression_stmt *stmt = container_of(ent, struct bae_expression_stmt, ent);
    gen_bcc_ast_entry(state, stmt->expression);
}

static void gen_node_if(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_if *if_op = container_of(ent, struct bae_if, ent);
    int if_count = state->next_label++;

    gen_bcc_ast_entry(state, if_op->if_expression);
    gen_out(state, "    cmp $0, %r\n", REG_ARG('a', bae_size(if_op->if_expression)));
    gen_out(state, "    je .L_ELSE_CLAUSE%d\n", if_count);

    gen_bcc_ast_entry(state, if_op->block);
    gen_out(state, "    jmp .L_END_IF%d\n", if_count);
    gen_out(state, ".L_ELSE_CLAUSE%d:\n", if_count);
    if (if_op->else_block)
        gen_bcc_ast_entry(state, if_op->else_block);
    gen_out(state, ".L_END_IF%d:\n", if_count);
}

static void gen_node_func_call(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_func_call *call = container_of(ent, struct bae_func_call, ent);
    struct bcc_ast_entry *entry;
    int param_size = 0;

    list_foreach_entry_reverse(&call->param_list, entry, entry) {
        gen_bcc_ast_entry(state, entry);
        if (bae_size(entry) <= 4) {
            gen_out(state, "    pushl %%eax\n");
            param_size += 4;
        }
    }

    gen_out(state, "    call %s\n", call->func->name);
    gen_out(state, "    addl $%d, %%esp\n", param_size);
}

static void gen_node_block(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_block *block = container_of(ent, struct bae_block, ent);
    struct bcc_ast_entry *entry;

    list_foreach_entry(&block->entry_list, entry, entry)
        gen_bcc_ast_entry(state, entry);
}

static void gen_node_assign(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_assign *assign = container_of(ent, struct bae_assign, ent);

    if (assign->optional_expr)
        gen_bcc_ast_entry(state, assign->optional_expr);

    gen_bcc_ast_entry(state, assign->rvalue);
    gen_bcc_ast_store(state, assign->lvalue);
}

static void gen_node_return(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_return *ret = container_of(ent, struct bae_return, ent);
    gen_bcc_ast_entry(state, ret->ret_value);

    /* Return value already in eax */
    gen_out(state, "    leave\n");
    gen_out(state, "    ret\n");
}

static void gen_node_while(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_while *w = container_of(ent, struct bae_while, ent);
    int label = state->next_label++;

    gen_out(state, ".L_WHILE_CONDITION%d:\n", label);

    gen_bcc_ast_entry(state, w->condition);
    gen_out(state, "    cmpl $0, %%eax\n");
    gen_out(state, "    je .L_WHILE_END%d\n", label);

    gen_bcc_ast_entry(state, w->block);
    gen_out(state, "    jmp .L_WHILE_CONDITION%d\n", label);

    gen_out(state, ".L_WHILE_END%d:\n", label);
}

static void gen_node_cast(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_cast *cast = container_of(ent, struct bae_cast, ent);
    struct bcc_ast_type *from = cast->expr->node_type, *to = cast->target;

    gen_bcc_ast_entry(state, cast->expr);

    if (bcc_ast_type_is_pointer(from) || bcc_ast_type_is_pointer(to))
        return ;

    if (bcc_ast_type_is_primitive(from) && bcc_ast_type_is_primitive(to)) {
        size_t size_from = from->size;
        size_t size_to = to->size;

        gen_conv_op(state, 'a', size_from, size_to);
    }
}

static void gen_node_var(struct gen_state *state, struct bcc_ast_entry *ent)
{
    struct bae_var *var = container_of(ent, struct bae_var, ent);
    gen_out(state, "    mov -%d(%%ebp), %r\n", var->var->loffset, REG_ARG('a', var->var->type->size));
}

static void (*gen_entry_table[BCC_AST_NODE_MAX]) (struct gen_state *state, struct bcc_ast_entry *ent) = {
    [BCC_AST_NODE_LITERAL_NUMBER] = gen_node_literal_number,
    [BCC_AST_NODE_LITERAL_STRING] = gen_node_literal_string,
    [BCC_AST_NODE_VAR_LOAD] = gen_node_var_load,
    [BCC_AST_NODE_VAR_STORE] = gen_node_var_store,
    [BCC_AST_NODE_BINARY_OP] = gen_node_binary_op,
    [BCC_AST_NODE_UNARY_OP] = gen_node_unary_op,
    [BCC_AST_NODE_EXPRESSION_STMT] = gen_node_expression_stmt,
    [BCC_AST_NODE_IF] = gen_node_if,
    [BCC_AST_NODE_FUNC_CALL] = gen_node_func_call,
    [BCC_AST_NODE_BLOCK] = gen_node_block,
    [BCC_AST_NODE_ASSIGN] = gen_node_assign,
    [BCC_AST_NODE_RETURN] = gen_node_return,
    [BCC_AST_NODE_WHILE] = gen_node_while,
    [BCC_AST_NODE_CAST] = gen_node_cast,
    [BCC_AST_NODE_VAR] = gen_node_var,
};

void gen_bcc_ast_entry(struct gen_state *state, struct bcc_ast_entry *ent)
{
    void (*gen_func) (struct gen_state *, struct bcc_ast_entry *);

    gen_func = gen_entry_table[ent->type];

    if (gen_func)
        (gen_func) (state, ent);
}

