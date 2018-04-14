
#include <stdlib.h>

#include "ast.h"
#include "parser.h"
#include "binop_create.h"
#include "assign_create.h"

struct bcc_ast_entry *create_assignment(struct bcc_ast *ast, struct bcc_parser_state *state, struct bcc_ast_entry *lvalue, struct bcc_ast_entry *rvalue, enum bcc_ast_binary_op bin_op)
{
    struct bcc_ast_variable *temp_var = NULL;
    struct bcc_ast_entry *optional_expr = NULL;

    if (bin_op != BCC_AST_BINARY_OP_MAX) {
        struct bae_var *v;
        struct bae_var *old;

        switch (lvalue->type) {
        case BCC_AST_NODE_VAR:
            old = container_of(lvalue, struct bae_var, ent);
            v = create_bae_var(ast);
            v->var = old->var;
            v->ent.node_type = old->ent.node_type;

            rvalue = create_bin_from_components(ast, bin_op, &v->ent, rvalue);
            if (!rvalue)
                return NULL;

            break;

        default:
            printf("Not compatible lvalue: %d\n", lvalue->type);
            return NULL;
        }
    /*
        struct bae_assign *a = create_bae_assign();
        struct bae_var *v = create_bae_var();
        temp_var = bae_function_get_temp_var(state->current_func);

        v->var = temp_var;
        v->ent.node_type = lvalue->node_type;

        a->rvalue = lvalue;
        a->lvalue = &v->ent;
        a->ent.node_type = lvalue->node_type;

        optional_expr = &a->ent;

        v = create_bae_var();
        v->var = temp_var;
        v->ent.node_type = lvalue->node_type;

        rvalue = create_bin_from_components(bin_op, &v->ent, rvalue);

        */

/*
        v = create_bae_var();
        v->var = temp_var;
        v->ent.node_type = lvalue->node_type;

        lvalue = &v->ent;
        */
    }

    rvalue = bcc_ast_entry_conv(ast, rvalue);

    if (!bcc_ast_type_lvalue_identical_to_rvalue(lvalue->node_type, rvalue->node_type)) {
        if (!bcc_ast_type_implicit_cast_exists(rvalue->node_type, lvalue->node_type))
            return false;

        struct bae_cast *cast = create_bae_cast(ast);
        cast->expr = rvalue;
        cast->target = lvalue->node_type;

        rvalue = &cast->ent;
    }

    struct bae_assign *assign = create_bae_assign(ast);
    assign->optional_expr = optional_expr;
    assign->lvalue = lvalue;
    assign->rvalue = rvalue;
    assign->ent.node_type = assign->lvalue->node_type;

    if (temp_var)
        bae_function_put_temp_var(state->current_func, temp_var);

    return &assign->ent;
}

