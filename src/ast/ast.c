
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "container_of.h"

#include "ast.h"
#include "in/recurse/parser.h"
#include "in/recurse/lexer.h"
#include "in/lalr/parser.h"
#include "in/lalr/lexer.h"
#include "out/gen.h"

static int parse_using_recurse_decent_parser(struct bcc_ast *ast, FILE *in)
{
    struct recurse_parse_state state;
    recurse_parse_state_init(&state);

    int ret = 0;
    recurse_in = in;
    state.in = in;
    state.ast = ast;

    struct lextok *tok;

    while ((tok = parser_peek(&state))->tok != TOK_EOF) {
        parser_invalid_token_error(&state);
        parser_consume(&state);
    }

    recurse_in = NULL;

    return ret;
}

static int parse_using_yacc(struct bcc_ast *ast, FILE *in)
{
    struct bcc_parser_state state;
    bcc_parser_state_init(&state);

    yyscan_t scan;

    yylex_init(&scan);
    yyset_in(in, scan);

    state.scanner = scan;
    state.ast = ast;

    int ret = yyparse(ast, &state, scan);

    yylex_destroy(scan);
    return ret;
}

int bcc_ast_parse(struct bcc_ast *ast, FILE *in)
{
    return parse_using_yacc(ast, in);
}

void bcc_ast_out(struct bcc_ast *ast, FILE *out, enum bcc_ast_out_format format)
{
    switch (format) {
    case BCC_AST_OUT_ASM_X86:
        gen_asm_x86(ast, out);
        break;

    case BCC_AST_OUT_TEXT:
        gen_text(ast, out);
        break;

    case BCC_AST_OUT_DUMP_AST:
        gen_dump_ast(ast, out);
        break;
    }
}

bool bcc_ast_entry_is_lvalue(struct bcc_ast_entry *ent)
{
    struct bae_unary_op *uop;

    switch (ent->type) {
    case BCC_AST_NODE_VAR:
        return true;

    case BCC_AST_NODE_UNARY_OP:
        uop = container_of(ent, struct bae_unary_op, ent);
        switch (uop->op) {
        case BCC_AST_UNARY_OP_DEREF:
            return true;

        default:
            return false;
        }

    default:
        return false;
    }
}

struct bcc_ast_variable *bcc_ast_find_variable(struct bcc_ast *ast, struct bae_function *func, struct bae_block *scope, const char *name)
{
    struct bcc_ast_variable *var;
    struct bae_block *current = scope;

    for (current = scope; current; current = current->outer_block)
        list_foreach_entry(&current->variable_list, var, block_entry)
            if (strcmp(var->name, name) == 0)
                return var;

    list_foreach_entry(&func->param_list, var, block_entry)
        if (strcmp(var->name, name) == 0)
            return var;

    return NULL;
}

struct bae_function *bcc_ast_find_function(struct bcc_ast *ast, const char *name)
{
    struct bae_function *func;

    list_foreach_entry(&ast->function_list, func, function_entry)
        if (strcmp(func->name, name) == 0)
            return func;

    return NULL;
}

void bcc_ast_add_function(struct bcc_ast *ast, struct bae_function *func)
{
    ast->function_count++;
    list_add_tail(&ast->function_list, &func->function_entry);
}

void bcc_ast_clear(struct bcc_ast *ast)
{
    struct bae_function *func;

    list_foreach_take_entry(&ast->function_list, func, function_entry) {
        bcc_ast_entry_clear(&func->ent);
        free(func);
    }

    object_pool_clear(&ast->type_object_pool);
}

void bcc_ast_add_literal_string(struct bcc_ast *ast, struct bae_literal_string *lit_str)
{
    lit_str->string_id = ast->next_string_id++;
    list_add_tail(&ast->literal_string_list, &lit_str->literal_string_entry);
}

