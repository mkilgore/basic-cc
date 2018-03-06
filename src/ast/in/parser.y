%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container_of.h"
#include "list.h"

#include "lexer.h"
#include "parser.h"

#include "ast.h"

void yyerror(struct bcc_ast *ast, struct bcc_parser_state *state, const char *str);

#define YYERROR_VERBOSE

static inline struct bae_literal_number *create_bae_literal_number(int val);
static inline struct bae_binary_op *create_bae_binary_op(enum bcc_ast_binary_op op);
static inline struct bae_expression_stmt *create_bae_expression_stmt(void);
static inline struct bae_if *create_bae_if(void);
static inline struct bae_block *create_bae_block(void);
static inline void bae_block_add_entry(struct bae_block *block, struct bcc_ast_entry *ent);
static inline struct bae_function *create_bae_function(char *name);
static inline struct bae_return *create_bae_return(void);
static inline struct bae_var_load *create_bae_var_load(void);
static inline struct bae_var_store *create_bae_var_store(void);
static inline struct bae_assign *create_bae_assign(void);
static inline struct bae_func_call *create_bae_func_call(void);
static inline struct bae_while *create_bae_while(void);
static inline struct bcc_ast_variable *bcc_ast_find_variable(struct bae_block *, const char *name);
static inline void bcc_ast_add_function(struct bcc_ast *ast, struct bae_function *func);
static inline struct bcc_ast_variable *create_bcc_ast_variable(void);
static inline struct bae_function *bcc_ast_find_function(struct bcc_ast *ast, const char *name);

struct temp_list {
    list_head_t head;   
};

#define TEMP_LIST_INIT(e) \
    { \
        .head = LIST_HEAD_INIT((e).head), \
    }

static inline void temp_list_init(struct temp_list *t)
{
    *t = (struct temp_list)TEMP_LIST_INIT(*t);
}
%}

%code provides {
#define YY_DECL \
    int yylex(struct bcc_ast *ast, struct bcc_parser_state *state)
YY_DECL;
}

%union {
    char *str;
    int ival;
    struct bcc_ast_entry *entry;
    struct bcc_ast_variable *param;
    struct temp_list *temp_list;
}

%parse-param { struct bcc_ast *ast } 
%lex-param { struct bcc_ast *ast } 
%parse-param { struct bcc_parser_state *state }
%lex-param { struct bcc_parser_state *state } 
%locations

%token <str> TOK_IDENT TOK_FUNCTION TOK_STRING
%token <ival> TOK_NUMBER

%token TOK_NOT_EQUAL TOK_GREATER_THAN_EQUAL TOK_LESS_THAN_EQUAL
%token TOK_EOF TOK_ERR
%token TOK_TYPE
%token TOK_RETURN
%token TOK_WHILE

%token TOK_IF TOK_ELSE

%type <entry> lvalue expression statement block optional_block function function_declaration
%type <param> parameter
%type <temp_list> function_arg_list function_arg_list_or_empty

%nonassoc "then"
%nonassoc TOK_ELSE
%nonassoc "load"
%nonassoc '='

%left '+' '-'
%left '*' '/'

%start basic_cc_file

%%

lvalue
    : TOK_IDENT {
        struct bae_var_store *store = create_bae_var_store();
        store->var = bcc_ast_find_variable(state->current_scope, $1);
        free($1);
        $$ = &store->ent;
    }

function_arg_list
    : expression {
        struct temp_list *lst = malloc(sizeof(*lst));
        temp_list_init(lst);
        list_add_tail(&lst->head, &$1->entry);
        $$ = lst;
    }
    | function_arg_list ',' expression {
        struct temp_list *lst = $1;
        list_add_tail(&lst->head, &$3->entry);
        $$ = lst;
    }

function_arg_list_or_empty
    : %empty { $$ = NULL; }
    | function_arg_list

expression
    : TOK_NUMBER {
        struct bae_literal_number *lit_num = create_bae_literal_number($1);
        $$ = &lit_num->ent;
    }
    | TOK_FUNCTION '(' function_arg_list_or_empty ')' {
        struct bae_func_call *call = create_bae_func_call();
        call->func = bcc_ast_find_function(ast, $1);

        if ($3) {
            list_replace(&call->param_list, &$3->head);
            free($3);
        }

        free($1);

        $$ = &call->ent;
    }
    | TOK_IDENT %prec "load" {
        struct bae_var_load *var = create_bae_var_load();
        var->var = bcc_ast_find_variable(state->current_scope, $1);
        free($1);
        $$ = &var->ent;
    }
    | expression '+' expression {
        struct bae_binary_op *bin_op = create_bae_binary_op(BCC_AST_BINARY_OP_PLUS);
        bin_op->left = $1;
        bin_op->right = $3;
        $$ = &bin_op->ent;
    }
    | expression '-' expression {
        struct bae_binary_op *bin_op = create_bae_binary_op(BCC_AST_BINARY_OP_MINUS);
        bin_op->left = $1;
        bin_op->right = $3;
        $$ = &bin_op->ent;
    }
    | expression '*' expression {
        struct bae_binary_op *bin_op = create_bae_binary_op(BCC_AST_BINARY_OP_MULT);
        bin_op->left = $1;
        bin_op->right = $3;
        $$ = &bin_op->ent;
    }
    | expression '/' expression {
        struct bae_binary_op *bin_op = create_bae_binary_op(BCC_AST_BINARY_OP_DIV);
        bin_op->left = $1;
        bin_op->right = $3;
        $$ = &bin_op->ent;
    }
    | lvalue '=' expression {
        struct bae_assign *assign = create_bae_assign();
        assign->lvalue = $1;
        assign->rvalue = $3;
        $$ = &assign->ent;
    }
    | '(' expression ')' {
        $$ = $2;
    }

declaration_optional_assignment
    : TOK_IDENT {
        struct bcc_ast_variable *var = create_bcc_ast_variable();
        var->name = $1;
        list_add_tail(&state->current_scope->variable_list, &var->block_entry);
        list_add_tail(&state->current_func->local_variable_list, &var->func_entry);
    }
    | TOK_IDENT '=' expression {
        struct bcc_ast_variable *var = create_bcc_ast_variable();
        var->name = $1;
        list_add_tail(&state->current_scope->variable_list, &var->block_entry);
        list_add_tail(&state->current_func->local_variable_list, &var->func_entry);

        /* Create a new assignment of the expression to the just created variable */
        struct bae_var_store *store = create_bae_var_store();
        store->var = var;

        struct bae_assign *assign = create_bae_assign();
        assign->lvalue = &store->ent;
        assign->rvalue = $3;

        struct bae_expression_stmt *stmt = create_bae_expression_stmt();
        stmt->expression = &assign->ent;

        bae_block_add_entry(state->current_scope, &stmt->ent);
    }

declaration_list
    : declaration_optional_assignment
    | declaration_list ',' declaration_optional_assignment

declaration
    : TOK_TYPE declaration_list ';'


statement
    : expression ';' {
        struct bae_expression_stmt *stmt = create_bae_expression_stmt();
        stmt->expression = $1;
        $$ = &stmt->ent;
    }
    | TOK_IF '(' expression ')' optional_block %prec "then" {
        struct bae_if *i = create_bae_if();
        i->if_expression = $3;
        i->block = $5;
        $$ = &i->ent;
    }
    | TOK_IF '(' expression ')' optional_block TOK_ELSE optional_block {
        struct bae_if *i = create_bae_if();
        i->if_expression = $3;
        i->block = $5;
        i->else_block = $7;
        $$ = &i->ent;
    }
    | TOK_WHILE '(' expression ')' optional_block {
        struct bae_while *w = create_bae_while();
        w->condition = $3;
        w->block = $5;
        $$ = &w->ent;
    }
    | TOK_RETURN expression ';' {
        struct bae_return *r = create_bae_return();
        r->ret_value = $2;
        $$ = &r->ent;
    }

optional_block
    : statement
    | '{' block '}' {
        struct bae_block *block = container_of($2, struct bae_block, ent);
        state->current_scope = block->outer_block;
        $$ = $2;
    }

block
    : %empty {
        struct bae_block *block = create_bae_block(); 
        block->outer_block = state->current_scope;
        state->current_scope = block;
        $$ = &block->ent;
    }
    | block statement {
        bae_block_add_entry(container_of($1, struct bae_block, ent), $2);
        $$ = $1;
    }
    | block declaration {
        /* Declarations are added to the current block at creation */
    }

parameter
    : TOK_TYPE TOK_IDENT {
        struct bcc_ast_variable *var = create_bcc_ast_variable();
        var->name = $2;
        $$ = var;
    }

parameter_list
    : parameter {
        list_add_tail(&state->temp_param_list, &$1->block_entry);
    }
    | parameter_list ',' parameter {
        list_add_tail(&state->temp_param_list, &$3->block_entry);
    }

parameter_list_or_empty
    : %empty
    | parameter_list

function_declaration
    : TOK_TYPE TOK_IDENT '(' parameter_list_or_empty ')' {
        struct bae_function *func = create_bae_function($2);
        func->ret_type = NULL;
        state->current_func = func;

        if (!list_empty(&state->temp_param_list)) {
            list_replace(&func->param_list, &state->temp_param_list);
            list_head_init(&state->temp_param_list);
        }

        /* We add the incomplete function at this point to
         * ensure the lexer will see it */
        bcc_ast_add_function(ast, func);
        $$ = &func->ent;
    }

function
    : function_declaration '{' block '}' {
        struct bae_function *func = container_of($1, struct bae_function, ent);
        func->block = $3;
        state->current_scope = NULL;
        state->current_func = NULL;
        $$ = &func->ent;
    }
    | function_declaration ';' {
        struct bae_function *func = container_of($1, struct bae_function, ent);
        func->block = NULL;
        state->current_scope = NULL;
        state->current_func = NULL;
        $$ = &func->ent;
    }

basic_cc_file
    : %empty
    | basic_cc_file function
    | basic_cc_file TOK_EOF {
        YYACCEPT;
    }

%%

static inline struct bae_literal_number *create_bae_literal_number(int val)
{
    struct bae_literal_number *lit_num = malloc(sizeof(*lit_num));
    bae_literal_number_init(lit_num, val);
    return lit_num;
}

static inline struct bae_binary_op *create_bae_binary_op(enum bcc_ast_binary_op op)
{
    struct bae_binary_op *bin_op = malloc(sizeof(*bin_op));
    bae_binary_op_init(bin_op, op);
    return bin_op;
}

static inline struct bae_expression_stmt *create_bae_expression_stmt(void)
{
    struct bae_expression_stmt *stmt = malloc(sizeof(*stmt));
    bae_expression_stmt_init(stmt);
    return stmt;
}

static inline struct bae_if *create_bae_if(void)
{
    struct bae_if *i = malloc(sizeof(*i));
    bae_if_init(i);
    return i;
}

static inline struct bae_block *create_bae_block(void)
{
    struct bae_block *block = malloc(sizeof(*block));
    bae_block_init(block);
    return block;
}

static inline void bae_block_add_entry(struct bae_block *block, struct bcc_ast_entry *ent)
{
    block->ent_count++;
    list_add_tail(&block->entry_list, &ent->entry);
}

static inline struct bae_function *create_bae_function(char *name)
{
    struct bae_function *func = malloc(sizeof(*func));
    bae_function_init(func, name);
    return func;
}

static inline struct bae_return *create_bae_return(void)
{
    struct bae_return *ret = malloc(sizeof(*ret));
    bae_return_init(ret);
    return ret;
}

static inline struct bae_var_load *create_bae_var_load(void)
{
    struct bae_var_load *var = malloc(sizeof(*var));
    bae_var_load_init(var);
    return var;
}

static inline struct bae_var_store *create_bae_var_store(void)
{
    struct bae_var_store *var = malloc(sizeof(*var));
    bae_var_store_init(var);
    return var;
}

static inline struct bae_assign *create_bae_assign(void)
{
    struct bae_assign *assign = malloc(sizeof(*assign));
    bae_assign_init(assign);
    return assign;
}

static inline struct bae_func_call *create_bae_func_call(void)
{
    struct bae_func_call *call = malloc(sizeof(*call));
    bae_func_call_init(call);
    return call;
}

static inline struct bae_while *create_bae_while(void)
{
    struct bae_while *w = malloc(sizeof(*w));
    bae_while_init(w);
    return w;
}

static inline struct bcc_ast_variable *bcc_ast_find_variable(struct bae_block *scope, const char *name)
{
    struct bcc_ast_variable *var;
    struct bae_block *current = scope;

    for (current = scope; current; current = current->outer_block)
        list_foreach_entry(&current->variable_list, var, block_entry)
            if (strcmp(var->name, name) == 0)
                return var;

    return NULL;
}

static inline struct bae_function *bcc_ast_find_function(struct bcc_ast *ast, const char *name)
{
    struct bae_function *func;

    list_foreach_entry(&ast->function_list, func, function_entry)
        if (strcmp(func->name, name) == 0)
            return func;

    return NULL;
}

static inline void bcc_ast_add_function(struct bcc_ast *ast, struct bae_function *func)
{
    ast->function_count++;
    list_add_tail(&ast->function_list, &func->function_entry);
}

static inline struct bcc_ast_variable *create_bcc_ast_variable(void)
{
    struct bcc_ast_variable *var = malloc(sizeof(*var));
    bcc_ast_variable_init(var);
    return var;
}

void yyerror(struct bcc_ast *ast, struct bcc_parser_state *state, const char *str)
{
    fprintf(stderr, "Parser error: %d: %s\n", yylloc.first_line, str);
}

