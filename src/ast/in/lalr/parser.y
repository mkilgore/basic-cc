%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container_of.h"
#include "list.h"

#include "lexer.h"
#include "parser.h"

#include "ast.h"

#define YYERROR_VERBOSE

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

#define ABORT_WITH_ERROR(tok, str) \
    do { \
        parser_error((tok), scanner, (str)); \
        YYABORT; \
    } while (0)

static struct bcc_ast_entry *create_bin_from_components(enum bcc_ast_binary_op op, struct bcc_ast_entry *left, struct bcc_ast_entry *right);
static struct bcc_ast_entry *create_assignment(struct bcc_ast_entry *lvalue, struct bcc_ast_entry *rvalue, enum bcc_ast_binary_op bin_op);

#define ASSIGNMENT(lval, rval, loc, bin_op) \
    ({ \
        struct bcc_ast_entry *assignment = create_assignment((lval), (rval), (bin_op)); \
        if (!assignment) { \
            char *str1, *str2; \
            char buf[2048]; \
            str1 = bcc_ast_type_get_name(lval->node_type); \
            str2 = bcc_ast_type_get_name(rval->node_type); \
            snprintf(buf, sizeof(buf), "Type of lvalue (%s) not compatible with type of rvalue (%s)", str1, str2); \
            free(str1); \
            free(str2); \
            ABORT_WITH_ERROR(&(loc), buf); \
        } \
        assignment; \
    })

%}

%code requires {
typedef struct YYLTYPE {
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  off_t file_line_offset;
} YYLTYPE;
# define YYLTYPE_IS_DECLARED 1 /* alert the parser that we have our own definition */

# define YYLLOC_DEFAULT(Current, Rhs, N)                                   \
    do                                                                     \
      if (N)                                                               \
        {                                                                  \
          (Current).first_line       = YYRHSLOC (Rhs, 1).first_line;       \
          (Current).first_column     = YYRHSLOC (Rhs, 1).first_column;     \
          (Current).last_line        = YYRHSLOC (Rhs, N).last_line;        \
          (Current).last_column      = YYRHSLOC (Rhs, N).last_column;      \
          (Current).file_line_offset = YYRHSLOC (Rhs, 1).file_line_offset; \
        }                                                                  \
      else                                                                 \
        { /* empty RHS */                                                  \
          (Current).first_line   = (Current).last_line   =                 \
            YYRHSLOC (Rhs, 0).last_line;                                   \
          (Current).first_column = (Current).last_column =                 \
            YYRHSLOC (Rhs, 0).last_column;                                 \
          (Current).file_line_offset = YYRHSLOC(Rhs, 0).file_line_offset;  \
        }                                                                  \
    while (0)
}

%code provides {
#define YY_DECL \
    int yylex(YYSTYPE *yylval_param, YYLTYPE *yylloc_param, struct bcc_ast *ast, struct bcc_parser_state *state, yyscan_t yyscanner)
YY_DECL;

void parser_error(YYLTYPE *loc, yyscan_t scanner, const char *str);
void parser_warning(YYLTYPE *loc, yyscan_t scanner, const char *str);

void yyerror(YYLTYPE *, struct bcc_ast *ast, struct bcc_parser_state *state, yyscan_t, const char *str);
}

%union {
    char *str;
    int ival;
    struct bcc_ast_entry *entry;
    struct bcc_ast_variable *param;
    struct temp_list *temp_list;
    struct bcc_ast_type *type;

    struct {
        struct bcc_ast_type *type;
        char *ident;
    } declarator;
}

%define api.pure full

%parse-param { struct bcc_ast *ast }
%lex-param { struct bcc_ast *ast }
%parse-param { struct bcc_parser_state *state }
%lex-param { struct bcc_parser_state *state }
%parse-param { yyscan_t scanner }
%lex-param { yyscan_t scanner }

%locations

%token <str> TOK_IDENT TOK_FUNCTION TOK_STRING
%token <ival> TOK_NUMBER

%token TOK_NOT_EQUAL          "!="
%token TOK_GREATER_THAN_EQUAL ">="
%token TOK_LESS_THAN_EQUAL    "<="
%token TOK_DOUBLE_EQUAL       "=="
%token TOK_SHIFTRIGHT         ">>"
%token TOK_SHIFTLEFT          "<<"
%token TOK_LOGICAL_AND        "&&"
%token TOK_LOGICAL_OR         "||"
%token TOK_PLUSPLUS           "++"
%token TOK_MINUSMINUS         "--"
%token TOK_EOF
%token TOK_ERR
%token <type> TOK_TYPE
%token TOK_RETURN
%token TOK_WHILE
%token TOK_ELLIPSIS

%token TOK_ASSIGN_PLUS "+="
%token TOK_ASSIGN_MINUS "-="
%token TOK_ASSIGN_MULT "*="
%token TOK_ASSIGN_DIV "/="
%token TOK_ASSIGN_MOD "%="
%token TOK_ASSIGN_SHIFTRIGHT ">>="
%token TOK_ASSIGN_SHIFTLEFT "<<="
%token TOK_ASSIGN_AND "&="
%token TOK_ASSIGN_OR  "|="
%token TOK_ASSIGN_XOR "^="

%token TOK_INT TOK_CHAR TOK_LONG TOK_SHORT

%token TOK_IF TOK_ELSE

%type <entry> lvalue expression statement block optional_block function function_declaration
%type <param> parameter

%type <ival> parameter_list_optional_ellipsis parameter_list_or_empty

%type <temp_list> function_arg_list
%type <temp_list> function_arg_list_or_empty
%type <str> string

%type <type> type_specifier type_direct_abstract_declaractor type_abstract_declarator
%type <type> type_pointer
%type <declarator> type_direct_declarator type_declarator type_name

%nonassoc "then"
%nonassoc TOK_ELSE
%nonassoc "load"

%left ','
%right "assignment" /* All assignment statements have the same precedence */
%right "ternary"
%left TOK_LOGICAL_OR
%left TOK_LOGICAL_AND
%left '|'
%left '^'
%left '&'
%left TOK_DOUBLE_EQUAL TOK_NOT_EQUAL
%left '>' '<' TOK_GREATER_THAN_EQUAL TOK_LESS_THAN_EQUAL
%left TOK_SHIFTRIGHT TOK_SHIFTLEFT
%left '+' '-'
%left '*' '/' '%'
%right '!' '~' TOK_PLUSPLUS TOK_MINUSMINUS "unary_plus" "unary_minus" "dereference" "addressof" "cast"
%left "plusplus-postfix" "minusminus-postfix"

%start basic_cc_file

%%

/* Type parsing */
type_specifier
    : TOK_INT   { $$ = bcc_ast_type_primitives + BCC_AST_PRIM_INT; }
    | TOK_LONG  { $$ = bcc_ast_type_primitives + BCC_AST_PRIM_LONG; }
    | TOK_SHORT { $$ = bcc_ast_type_primitives + BCC_AST_PRIM_SHORT; }
    | TOK_CHAR  { $$ = bcc_ast_type_primitives + BCC_AST_PRIM_CHAR; }

type_pointer
    : '*' {
        $$ = create_bcc_ast_type_pointer(ast, NULL);
    }
    | type_pointer '*' {
        $$ = create_bcc_ast_type_pointer(ast, $1);
    }

type_direct_abstract_declaractor
    : '(' type_abstract_declarator ')' {
        $$ = $2;
    }
    | '(' ')' {
        $$ = NULL;
    }

type_abstract_declarator
    : type_pointer
    | type_direct_abstract_declaractor
    | type_pointer type_direct_abstract_declaractor {
        struct bcc_ast_type *type = $1;

        for (; type->inner; type = type->inner)
            ;

        type->inner = $2;
        $$ = $1;
    }

type_direct_declarator
    : TOK_IDENT {
        $$.type = NULL;
        $$.ident = $1;
    }
    | '(' type_declarator ')' {
        $$ = $2;
    }

type_declarator
    : type_pointer type_direct_declarator {
        struct bcc_ast_type *type = $1;
        for (; type->inner; type = type->inner)
            ;

        type->inner = $2.type;
        $$ = $2;
    }
    | type_direct_declarator

/*
type_declaration
    : type_specifier type_declarator {
        struct bcc_ast_type *type = $2.type;
        for (; type->inner; type = type->inner)
            ;

        type->inner = $1;
        $$ = $2;
    }
    */

type_name
    : type_specifier  {
        $$.ident = NULL;
        $$.type = $1;
    }
    | type_specifier type_abstract_declarator {
        struct bcc_ast_type *type = $2;
        for (; type->inner; type = type->inner)
            ;

        type->inner = $1;
        $$.ident = NULL;
        $$.type = $2;
    }
    | type_specifier type_declarator {
        struct bcc_ast_type *type = $2.type;

        if ($2.type) {
            for (; type->inner; type = type->inner)
                ;

            type->inner = $1;
            $$ = $2;
        } else {
            $$.type = $1;
            $$.ident = $2.ident;
        }
    }

string
    : TOK_STRING
    | string TOK_STRING {
        size_t len = strlen($1) + strlen($2) + 1;
        char *newstr = realloc($1, len);

        strcat(newstr, $2);
        free($2);

        $$ = newstr;
    }

lvalue
    : TOK_IDENT {
        struct bae_var_store *store = create_bae_var_store();
        store->var = bcc_ast_find_variable(ast, state->current_scope, $1);
        store->ent.node_type = store->var->type;
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
    : '(' expression ')' { $$ = $2; }
    | TOK_NUMBER {
        struct bae_literal_number *lit_num = create_bae_literal_number($1);
        lit_num->ent.node_type = bcc_ast_type_primitives + BCC_AST_PRIM_INT;
        $$ = &lit_num->ent;
    }
    | string {
        struct bae_literal_string *lit_str = create_bae_literal_string();
        lit_str->ent.node_type = &bcc_ast_type_char_ptr;
        lit_str->str = $1;
        $$ = &lit_str->ent;
    }
    | TOK_FUNCTION '(' function_arg_list_or_empty ')' {
        struct bae_func_call *call = create_bae_func_call();
        call->func = bcc_ast_find_function(ast, $1);
        call->ent.node_type = call->func->ret_type;

        if ($3) {
            list_replace(&call->param_list, &$3->head);
            free($3);
        }

        free($1);

        $$ = &call->ent;
    }
    | TOK_IDENT %prec "load" {
        struct bae_var_load *var = create_bae_var_load();
        var->var = bcc_ast_find_variable(ast, state->current_scope, $1);
        if (!var->var)
            ABORT_WITH_ERROR(&@1, "Unknown variable name");

        var->ent.node_type = var->var->type;
        free($1);
        $$ = &var->ent;
    }
    | expression '+' expression  { $$ = create_bin_from_components(BCC_AST_BINARY_OP_PLUS, $1, $3); }
    | expression '-' expression  { $$ = create_bin_from_components(BCC_AST_BINARY_OP_MINUS, $1, $3); }
    | expression '*' expression  { $$ = create_bin_from_components(BCC_AST_BINARY_OP_MULT, $1, $3); }
    | expression '/' expression  { $$ = create_bin_from_components(BCC_AST_BINARY_OP_DIV, $1, $3); }
    | expression '%' expression  { $$ = create_bin_from_components(BCC_AST_BINARY_OP_MOD, $1, $3); }
    | expression ">>" expression { $$ = create_bin_from_components(BCC_AST_BINARY_OP_SHIFTRIGHT, $1, $3); }
    | expression "<<" expression { $$ = create_bin_from_components(BCC_AST_BINARY_OP_SHIFTLEFT, $1, $3); }
    | expression '>' expression  { $$ = create_bin_from_components(BCC_AST_BINARY_OP_GREATER_THAN, $1, $3); }
    | expression '<' expression  { $$ = create_bin_from_components(BCC_AST_BINARY_OP_LESS_THAN, $1, $3); }
    | expression ">=" expression { $$ = create_bin_from_components(BCC_AST_BINARY_OP_GREATER_THAN_EQUAL, $1, $3); }
    | expression "<=" expression { $$ = create_bin_from_components(BCC_AST_BINARY_OP_LESS_THAN_EQUAL, $1, $3); }
    | expression "==" expression { $$ = create_bin_from_components(BCC_AST_BINARY_OP_DOUBLEEQUAL, $1, $3); }
    | expression "!=" expression { $$ = create_bin_from_components(BCC_AST_BINARY_OP_NOT_EQUAL, $1, $3); }
    | expression '&' expression  { $$ = create_bin_from_components(BCC_AST_BINARY_OP_BITWISE_AND, $1, $3); }
    | expression '|' expression  { $$ = create_bin_from_components(BCC_AST_BINARY_OP_BITWISE_OR, $1, $3); }
    | expression '^' expression  { $$ = create_bin_from_components(BCC_AST_BINARY_OP_BITWISE_XOR, $1, $3); }
    | expression "&&" expression { $$ = create_bin_from_components(BCC_AST_BINARY_OP_LOGICAL_AND, $1, $3); }
    | expression "||" expression { $$ = create_bin_from_components(BCC_AST_BINARY_OP_LOGICAL_OR, $1, $3); }
    | lvalue '=' expression %prec "assignment"   { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_MAX); }
    | lvalue "+=" expression %prec "assignment"  { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_PLUS); }
    | lvalue "-=" expression %prec "assignment"  { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_MINUS); }
    | lvalue "*=" expression %prec "assignment"  { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_MULT); }
    | lvalue "/=" expression %prec "assignment"  { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_DIV); }
    | lvalue "%=" expression %prec "assignment"  { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_MOD); }
    | lvalue ">>=" expression %prec "assignment" { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_SHIFTRIGHT); }
    | lvalue "<<=" expression %prec "assignment" { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_SHIFTLEFT); }
    | lvalue "&=" expression %prec "assignment"  { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_BITWISE_AND); }
    | lvalue "|=" expression %prec "assignment"  { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_BITWISE_OR); }
    | lvalue "^=" expression %prec "assignment"  { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_BITWISE_XOR); }

declaration_optional_assignment
    : type_declarator {
        struct bcc_ast_variable *var = create_bcc_ast_variable();
        struct bcc_ast_type *type;

        var->type = $1.type;
        var->name = $1.ident;

        /* Fill in the declaration type */
        if (var->type) {
            for (type = var->type; type->inner; type = type->inner)
                ;

            type->inner = state->declaration_type;
        } else {
            var->type = state->declaration_type;
        }

        list_add_tail(&state->current_scope->variable_list, &var->block_entry);
        list_add_tail(&state->current_func->local_variable_list, &var->func_entry);
    }
    | type_declarator '=' expression {
        struct bcc_ast_variable *var = create_bcc_ast_variable();
        struct bcc_ast_type *type;

        var->type = $1.type;
        var->name = $1.ident;

        /* Fill in the declaration type */
        if (var->type) {
            for (type = var->type; type->inner; type = type->inner)
                ;

            type->inner = state->declaration_type;
        } else {
            var->type = state->declaration_type;
        }

        list_add_tail(&state->current_scope->variable_list, &var->block_entry);
        list_add_tail(&state->current_func->local_variable_list, &var->func_entry);

        /* Create a new assignment of the expression to the just created variable */
        struct bae_var_store *store = create_bae_var_store();
        store->var = var;
        store->ent.node_type = var->type;

        struct bae_assign *assign = create_bae_assign();
        assign->lvalue = &store->ent;
        assign->rvalue = $3;
        assign->ent.node_type = store->ent.node_type;

        struct bae_expression_stmt *stmt = create_bae_expression_stmt();
        stmt->expression = &assign->ent;

        bae_block_add_entry(state->current_scope, &stmt->ent);
    }

declaration_list
    : declaration_optional_assignment
    | declaration_list ',' declaration_optional_assignment

declaration
    : type_specifier { state->declaration_type = $1; } declaration_list ';'


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
    : type_name {
        struct bcc_ast_variable *var = create_bcc_ast_variable();
        var->type = $1.type;
        var->name = $1.ident;
        $$ = var;
    }

parameter_list
    : parameter {
        list_add_tail(&state->temp_param_list, &$1->block_entry);
    }
    | parameter_list ',' parameter {
        list_add_tail(&state->temp_param_list, &$3->block_entry);
    }

parameter_list_optional_ellipsis
    : parameter_list { $$ = 0; }
    | parameter_list ',' TOK_ELLIPSIS { $$ = 1; }

parameter_list_or_empty
    : %empty { $$ = 0; }
    | parameter_list_optional_ellipsis

function_declaration
    : type_specifier TOK_IDENT '(' parameter_list_or_empty ')' {
        struct bae_function *func = create_bae_function($2);
        func->ret_type = $1;
        func->has_ellipsis = $4;
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

static struct bcc_ast_entry *create_bin_from_components(enum bcc_ast_binary_op op, struct bcc_ast_entry *left, struct bcc_ast_entry *right)
{
    struct bae_binary_op *bin_op = create_bae_binary_op(op);
    bin_op->left = left;
    bin_op->right = right;

    /* DO TYPE COMPATIBILITY CHECK AND UPCASTING HERE */
    bin_op->ent.node_type = bin_op->left->node_type;
    return &bin_op->ent;
}

static struct bcc_ast_entry *create_assignment(struct bcc_ast_entry *lvalue, struct bcc_ast_entry *rvalue, enum bcc_ast_binary_op bin_op)
{
    if (bin_op != BCC_AST_BINARY_OP_MAX) {
        struct bcc_ast_entry *val = bcc_ast_convert_to_rvalue(lvalue);
        if (!val)
            return NULL;

        rvalue = create_bin_from_components(bin_op, val, rvalue);
    }

    struct bae_assign *assign = create_bae_assign();
    assign->lvalue = lvalue;
    assign->rvalue = rvalue;

    if (!bcc_ast_type_are_identical(lvalue->node_type, rvalue->node_type))
        return NULL;

    assign->ent.node_type = assign->lvalue->node_type;
    return &assign->ent;
}

static void display_invalid_token(YYLTYPE *loc, yyscan_t scanner)
{
    FILE *in = yyget_in(scanner);
    off_t sav = ftell(in);

    fseek(in, loc->file_line_offset, SEEK_SET);

    char *buf = NULL;
    size_t len = 0;
    getline(&buf, &len, in);

    fprintf(stderr, "%s", buf);
    free(buf);

    int i;
    for (i = 0; i < loc->first_column; i++)
        fputc(' ', stderr);

    fputc('^', stderr);
    i++;

    for (; i < loc->last_column; i++)
        fputc('-', stderr);

    if (loc->last_column > loc->first_column)
        fputc('^', stderr);

    fputc('\n', stderr);

    fseek(in, sav, SEEK_SET);
}

void yyerror(YYLTYPE *loc, struct bcc_ast *ast, struct bcc_parser_state *state, yyscan_t scanner, const char *str)
{
    parser_error(loc, scanner, str);
}

void parser_error(YYLTYPE *loc, yyscan_t scanner, const char *str)
{
    fprintf(stderr, "Error: %s at:\n", str);
    display_invalid_token(loc, scanner);
}

void parser_warning(YYLTYPE *loc, yyscan_t scanner, const char *str)
{
    fprintf(stderr, "Warning: %s at:\n", str);
    display_invalid_token(loc, scanner);
}

