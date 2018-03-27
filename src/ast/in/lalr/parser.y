%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container_of.h"
#include "list.h"
#include "bits.h"

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

static struct bcc_ast_entry *create_bin_from_components(struct bcc_ast *ast, enum bcc_ast_binary_op op, struct bcc_ast_entry *left, struct bcc_ast_entry *right);
static struct bcc_ast_entry *create_assignment(struct bcc_ast *ast, struct bcc_parser_state *, struct bcc_ast_entry *lvalue, struct bcc_ast_entry *rvalue, enum bcc_ast_binary_op bin_op);

enum bcc_ast_type_specifier {
    BCC_AST_TYPE_SPECIFIER_INT,
    BCC_AST_TYPE_SPECIFIER_LONG,
    BCC_AST_TYPE_SPECIFIER_UNSIGNED,
    BCC_AST_TYPE_SPECIFIER_SIGNED,
    BCC_AST_TYPE_SPECIFIER_SHORT,
    BCC_AST_TYPE_SPECIFIER_CHAR,
    BCC_AST_TYPE_SPECIFIER_VOID,
};

static bool specifier_is_invalid(flags_t specifiers, int new_specifier);
static void specifier_add_to_type(struct bcc_ast_type *type, int specifier);

static inline enum bcc_ast_primitive_type specifier_to_prim(enum bcc_ast_type_specifier spec)
{
    switch (spec) {
    case BCC_AST_TYPE_SPECIFIER_INT:
        return BCC_AST_PRIM_INT;

    case BCC_AST_TYPE_SPECIFIER_CHAR:
        return BCC_AST_PRIM_CHAR;

    case BCC_AST_TYPE_SPECIFIER_SHORT:
        return BCC_AST_PRIM_SHORT;

    case BCC_AST_TYPE_SPECIFIER_LONG:
        return BCC_AST_PRIM_LONG;

    case BCC_AST_TYPE_SPECIFIER_VOID:
        return BCC_AST_PRIM_VOID;

    default:
        return BCC_AST_PRIM_MAX;
    }
}

#define ASSIGNMENT(lval, rval, loc, bin_op) \
    ({ \
        if (bcc_ast_type_is_const((lval)->node_type)) \
            ABORT_WITH_ERROR(&(loc), "lvalue is const"); \
            \
        struct bcc_ast_entry *assignment = create_assignment(ast, state, (lval), (rval), (bin_op)); \
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

#define BIN_OP(op, lval, rval, loc) \
    ({ \
        struct bcc_ast_entry *bin_op = create_bin_from_components(ast, (op), (lval), (rval)); \
        if (!bin_op) { \
            char *str1, *str2; \
            char buf[2048]; \
            str1 = bcc_ast_type_get_name(lval->node_type); \
            str2 = bcc_ast_type_get_name(rval->node_type); \
            snprintf(buf, sizeof(buf), "Type of left expression (%s) not compatible with type of right expression (%s)", str1, str2); \
            free(str1); \
            free(str2); \
            ABORT_WITH_ERROR(&(loc), buf); \
        } \
        bin_op; \
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

    struct {
        struct bcc_ast_type *type;
        flags_t specifier_flags;
    } type_base;
}

%define api.pure full

%parse-param { struct bcc_ast *ast }
%lex-param { struct bcc_ast *ast }
%parse-param { struct bcc_parser_state *state }
%lex-param { struct bcc_parser_state *state }
%parse-param { yyscan_t scanner }
%lex-param { yyscan_t scanner }

%locations

%token <str> TOK_IDENT TOK_STRING
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

%token TOK_ASSIGN_PLUS       "+="
%token TOK_ASSIGN_MINUS      "-="
%token TOK_ASSIGN_MULT       "*="
%token TOK_ASSIGN_DIV        "/="
%token TOK_ASSIGN_MOD        "%="
%token TOK_ASSIGN_SHIFTRIGHT ">>="
%token TOK_ASSIGN_SHIFTLEFT  "<<="
%token TOK_ASSIGN_AND        "&="
%token TOK_ASSIGN_OR         "|="
%token TOK_ASSIGN_XOR        "^="

%token TOK_CONST TOK_UNSIGNED TOK_SIGNED
%token TOK_INT TOK_CHAR TOK_LONG TOK_SHORT TOK_VOID

%token TOK_IF TOK_ELSE

%type <entry> expression statement block optional_block function function_declaration
%type <entry> assignment_expression inner_expression unary_expression unary_postfix_expression paren_expression

%type <param> parameter

%type <ival> parameter_list_optional_ellipsis parameter_list_or_empty

%type <temp_list> function_arg_list
%type <temp_list> function_arg_list_or_empty
%type <str> string

%type <ival> type_specifier type_qualifier

%type <type_base> type_base_with_spec

%type <type> type_base type_direct_abstract_declaractor type_abstract_declarator
%type <type> type_pointer
%type <type> type_name
%type <declarator> type_direct_declarator type_declarator type_name_optional_ident

/* %type <bin_op> assignment_operator */

%precedence "then"
%precedence TOK_ELSE
/* %nonassoc "load" */

/* %left ',' */
%right '=' "+=" "-=" "*=" "/=" "%=" "&=" "|=" "^=" ">>=" "<<=" /* All assignment statements have the same precedence */
/* %right "ternary" */
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
/* %right '!' '~' "plusplus-prefix" "minusminus-prefix" "unary_plus" "unary_minus" "dereference" "addressof" "cast" */
/* %left "plusplus-postfix" "minusminus-postfix" "function-call" */

%start basic_cc_file

%%

/* Type parsing */
type_specifier
    : TOK_INT      { $$ = BCC_AST_TYPE_SPECIFIER_INT; }
    | TOK_LONG     { $$ = BCC_AST_TYPE_SPECIFIER_LONG; }
    | TOK_SHORT    { $$ = BCC_AST_TYPE_SPECIFIER_SHORT; }
    | TOK_CHAR     { $$ = BCC_AST_TYPE_SPECIFIER_CHAR; }
    | TOK_VOID     { $$ = BCC_AST_TYPE_SPECIFIER_VOID; }
    | TOK_UNSIGNED { $$ = BCC_AST_TYPE_SPECIFIER_UNSIGNED; }
    | TOK_SIGNED   { $$ = BCC_AST_TYPE_SPECIFIER_SIGNED; }

type_qualifier
    : TOK_CONST { $$ = BCC_AST_TYPE_QUALIFIER_CONST; }

/* type specifier combined with a type qualifier
 * The parsed specifier information is stored and passed separate, as well as applied to the struct bcc_ast_type object created.
 *
 * 'type_base' discards the extra specifier information after the type is fully parsed
 */
type_base_with_spec
    : type_specifier {
        $$.type = create_bcc_ast_type_prim(ast, BCC_AST_PRIM_MAX);
        $$.specifier_flags = F($1);
        specifier_add_to_type($$.type, $1);
    }
    | type_qualifier {
        struct bcc_ast_type *type = create_bcc_ast_type_prim(ast, BCC_AST_PRIM_MAX);
        flag_set(&type->qualifier_flags, $1);
        $$.type = type;
        $$.specifier_flags = 0;
    }
    | type_base_with_spec type_specifier {
        if (flag_test(&$1.specifier_flags, $2))
            ABORT_WITH_ERROR(&@2, "Duplicate specifier");

        if (specifier_is_invalid($1.specifier_flags, $2))
            ABORT_WITH_ERROR(&@2, "Invalid specifier");

        flag_set(&$1.specifier_flags, $2);
        specifier_add_to_type($1.type, $2);
        $$ = $1;
    }
    | type_base_with_spec type_qualifier {
        if (flag_test(&$1.type->qualifier_flags, $2))
            ABORT_WITH_ERROR(&@2, "Duplicate qualifier");

        flag_set(&$1.type->qualifier_flags, $2);
        $$ = $1;
    }

type_base
    : type_base_with_spec { $$ = $1.type; }

type_pointer
    : '*' {
        $$ = create_bcc_ast_type_pointer(ast, NULL);
    }
    | type_pointer type_qualifier {
        if (flag_test(&$1->qualifier_flags, $2))
            ABORT_WITH_ERROR(&@2, "Duplicate qualifier");

        flag_set(&$1->qualifier_flags, $2);
        $$ = $1;
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
        $$.type = $1;
        $$.ident = $2.ident;
    }
    | type_direct_declarator

/*
type_declaration
    : type_base type_declarator {
        struct bcc_ast_type *type = $2.type;
        for (; type->inner; type = type->inner)
            ;

        type->inner = $1;
        $$ = $2;
    }
    */

type_name
    : type_base {
        $$ = $1;
    }
    | type_base type_abstract_declarator {
        struct bcc_ast_type *type = $2;
        for (; type->inner; type = type->inner)
            ;

        type->inner = $1;
        $$ = $2;
    }

type_name_optional_ident
    : type_name { $$.ident = NULL; $$.type = $1; }
    | type_base type_declarator {
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

paren_expression
    : '(' expression ')' { $$ = $2; }
    | TOK_IDENT {
        struct bae_var *var = create_bae_var(ast);
        var->var = bcc_ast_find_variable(ast, state->current_func, state->current_scope, $1);

        if (!var->var)
            ABORT_WITH_ERROR(&@1, "Unknown variable name");

        var->ent.node_type = var->var->type;
        free($1);
        $$ = &var->ent;
    }
    | TOK_NUMBER {
        struct bae_literal_number *lit_num = create_bae_literal_number(ast, $1);
        lit_num->ent.node_type = &bcc_ast_type_int_zero;
        $$ = &lit_num->ent;
    }
    | string {
        struct bae_literal_string *lit_str = create_bae_literal_string(ast);
        lit_str->ent.node_type = &bcc_ast_type_char_ptr;
        lit_str->str = $1;
        bcc_ast_add_literal_string(ast, lit_str);
        $$ = &lit_str->ent;
    }

unary_postfix_expression
    : paren_expression
    | unary_postfix_expression "--" %prec "minusminus-postfix" {
        if (!bcc_ast_entry_is_lvalue($1))
            ABORT_WITH_ERROR(&@1, "Left of decrement is not an lvalue");

        struct bae_unary_op *op = create_bae_unary_op(ast, BCC_AST_UNARY_OP_MINUSMINUS_POSTFIX);
        op->lvalue = $1;
        op->expr = op->lvalue;
        op->ent.node_type = $1->node_type;
        $$ = &op->ent;
    }
    | unary_postfix_expression "++" %prec "plusplus-postfix" {
        if (!bcc_ast_entry_is_lvalue($1))
            ABORT_WITH_ERROR(&@1, "Left of increment is not an lvalue");

        struct bae_unary_op *op = create_bae_unary_op(ast, BCC_AST_UNARY_OP_PLUSPLUS_POSTFIX);
        op->lvalue = $1;
        op->expr = op->lvalue;
        op->ent.node_type = $1->node_type;
        $$ = &op->ent;
    }
    | unary_postfix_expression '[' expression ']' {
        if (!bcc_ast_type_is_pointer($1->node_type))
            ABORT_WITH_ERROR(&@1, "Invalid Unary");

        struct bcc_ast_entry *bin_op = create_bin_from_components(ast, BCC_AST_BINARY_OP_PLUS, $1, $3);
        if (!bin_op)
            ABORT_WITH_ERROR(&@3, "Invalid index");

        struct bae_unary_op *uop = create_bae_unary_op(ast, BCC_AST_UNARY_OP_DEREF);
        uop->expr = bin_op;
        uop->ent.node_type = bin_op->node_type->inner;
        $$ = &uop->ent;
    }
    | TOK_IDENT '(' function_arg_list_or_empty ')' {
        struct bae_function *func = bcc_ast_find_function(ast, $1);
        struct bcc_ast_variable *param;
        struct bcc_ast_entry *arg;

        if (!$3) {
            if (!list_empty(&func->param_list))
                ABORT_WITH_ERROR(&@3, "Not enough arguments supplied to function");
        } else {
            /*
             * Type checking and casting for arguments
             * This looks horrible, but we're really just looping through two lists at a time.
             */
            for (param = list_first_entry(&func->param_list, struct bcc_ast_variable, block_entry),
                 arg   = list_first_entry(&$3->head, struct bcc_ast_entry, entry);
                 !list_ptr_is_head(&func->param_list, &param->block_entry) && !list_ptr_is_head(&$3->head, &arg->entry);
                 param = list_next_entry(param, block_entry),
                 arg   = list_next_entry(arg, entry)) {

                if (bcc_ast_type_lvalue_identical_to_rvalue(param->type, arg->node_type))
                    continue;

                if (bcc_ast_type_implicit_cast_exists(arg->node_type, param->type)) {
                    struct bae_cast *cast = create_bae_cast(ast);
                    cast->expr = arg;
                    cast->target = param->type;
                    cast->ent.node_type = cast->target;

                    list_replace(&cast->ent.entry, &arg->entry);

                    arg = &cast->ent;
                    continue;
                }

                ABORT_WITH_ERROR(&@3, "Argument with invalid type");
            }

            if (!list_ptr_is_head(&func->param_list, &param->block_entry))
                ABORT_WITH_ERROR(&@3, "Not enough arguments supplied to function");

            if (!list_ptr_is_head(&$3->head, &arg->entry) && !func->has_ellipsis)
                ABORT_WITH_ERROR(&@3, "Too many arguments supplied to function");

            for (;
                !list_ptr_is_head(&$3->head, &arg->entry);
                arg = list_next_entry(arg, entry)) {

                if (bcc_ast_type_is_integer(arg->node_type)) {
                    struct bae_cast *cast = create_bae_cast(ast);
                    cast->expr = arg;
                    cast->target = bcc_ast_type_primitives + BCC_AST_PRIM_INT;
                    cast->ent.node_type = cast->target;

                    list_replace(&cast->ent.entry, &arg->entry);

                    arg = &cast->ent;
                    continue;
                }
            }
        }

        struct bae_func_call *call = create_bae_func_call(ast);
        call->func = bcc_ast_find_function(ast, $1);
        call->ent.node_type = call->func->ret_type;

        if ($3) {
            list_replace(&call->param_list, &$3->head);
            free($3);
        }

        free($1);

        $$ = &call->ent;
    }
    

unary_expression
    : unary_postfix_expression
    | '-' unary_expression {
        struct bae_unary_op *op = create_bae_unary_op(ast, BCC_AST_UNARY_OP_MINUS);
        op->expr = $2;
        op->ent.node_type = $2->node_type;
        $$ = &op->ent;
    }
    | '+' unary_expression {
        struct bae_unary_op *op = create_bae_unary_op(ast, BCC_AST_UNARY_OP_PLUS);
        op->expr = $2;
        op->ent.node_type = $2->node_type;
        $$ = &op->ent;
    }
    | "--" unary_expression {
        if (!bcc_ast_entry_is_lvalue($2))
            ABORT_WITH_ERROR(&@1, "Right of decrement is not an lvalue");

        struct bae_unary_op *op = create_bae_unary_op(ast, BCC_AST_UNARY_OP_MINUSMINUS);
        op->lvalue = $2;
        op->expr = op->lvalue;
        op->ent.node_type = $2->node_type;
        $$ = &op->ent;
    }
    | "++" unary_expression {
        if (!bcc_ast_entry_is_lvalue($2))
            ABORT_WITH_ERROR(&@1, "Right of increment is not an lvalue");

        struct bae_unary_op *op = create_bae_unary_op(ast, BCC_AST_UNARY_OP_PLUSPLUS);
        op->lvalue = $2;
        op->expr = op->lvalue;
        op->ent.node_type = $2->node_type;
        $$ = &op->ent;
    }
    | '&' unary_expression {
        if ($2->type == BCC_AST_NODE_UNARY_OP) {
            struct bae_unary_op *uop = container_of($2, struct bae_unary_op, ent);
            if (uop->op == BCC_AST_UNARY_OP_DEREF) {
                $$ = uop->expr;
                free(uop);
            } else {
                ABORT_WITH_ERROR(&@1, "Right of address-of is not an lvalue");
            }
        } else {
            if (!bcc_ast_entry_is_lvalue($2))
                ABORT_WITH_ERROR(&@1, "Right of address-of is not an lvalue");

            struct bae_unary_op *op = create_bae_unary_op(ast, BCC_AST_UNARY_OP_ADDRESS_OF);
            op->lvalue = $2;
            op->ent.node_type = create_bcc_ast_type_pointer(ast, $2->node_type);
            $$ = &op->ent;
        }
    }
    | '!' unary_expression {
        struct bae_unary_op *op = create_bae_unary_op(ast, BCC_AST_UNARY_OP_NOT);
        op->expr = $2;
        op->ent.node_type = bcc_ast_type_primitives + BCC_AST_PRIM_INT;
        $$ = &op->ent;
    }
    | '~' unary_expression {
        struct bae_unary_op *op = create_bae_unary_op(ast, BCC_AST_UNARY_OP_BITWISE_NOT);
        op->expr = $2;
        op->ent.node_type = $2->node_type;
        $$ = &op->ent;
    }
    | '*' unary_expression {
        struct bae_unary_op *uop = create_bae_unary_op(ast, BCC_AST_UNARY_OP_DEREF);
        uop->expr = $2;
        uop->ent.node_type = $2->node_type->inner;
        $$ = &uop->ent;
    }
    | '(' type_name ')' unary_expression {
        struct bcc_ast_type *target = $2;
        struct bcc_ast_type *start = $4->node_type;
        struct bae_cast *cast;

        if (bcc_ast_type_is_integer(start) && bcc_ast_type_is_integer(target)) {
            /* Integer casts are always allowed */
        } else if (bcc_ast_type_is_pointer(start) && bcc_ast_type_is_pointer(target)) {
            /* Pointer casts are always allowed */
        } else if (bcc_ast_type_is_pointer(start) && bcc_ast_type_is_integer(target)) {
            /* Cast from pointer to integer */
            if (start->size != target->size)
                parser_warning(&@2, scanner, "Pointer is cast to integer of wrong size");

        } else if (bcc_ast_type_is_integer(start) && bcc_ast_type_is_pointer(target)) {
            /* Cast from integer to pointer */
        } else {
            ABORT_WITH_ERROR(&@2, "Invalid cast");
        }

        cast = create_bae_cast(ast);
        cast->expr = $4;
        cast->target = target;
        cast->ent.node_type = target;
        $$ = &cast->ent;
    }

inner_expression
    : unary_expression
    | inner_expression '+'  inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_PLUS, $1, $3, @1); }
    | inner_expression '-'  inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_MINUS, $1, $3, @1); }
    | inner_expression '*'  inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_MULT, $1, $3, @1); }
    | inner_expression '/'  inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_DIV, $1, $3, @1); }
    | inner_expression '%'  inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_MOD, $1, $3, @1); }
    | inner_expression ">>" inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_SHIFTRIGHT, $1, $3, @1); }
    | inner_expression "<<" inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_SHIFTLEFT, $1, $3, @1); }
    | inner_expression '>'  inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_GREATER_THAN, $1, $3, @1); }
    | inner_expression '<'  inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_LESS_THAN, $1, $3, @1); }
    | inner_expression ">=" inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_GREATER_THAN_EQUAL, $1, $3, @1); }
    | inner_expression "<=" inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_LESS_THAN_EQUAL, $1, $3, @1); }
    | inner_expression "==" inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_DOUBLEEQUAL, $1, $3, @1); }
    | inner_expression "!=" inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_NOT_EQUAL, $1, $3, @1); }
    | inner_expression '&'  inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_BITWISE_AND, $1, $3, @1); }
    | inner_expression '|'  inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_BITWISE_OR, $1, $3, @1); }
    | inner_expression '^'  inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_BITWISE_XOR, $1, $3, @1); }
    | inner_expression "&&" inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_LOGICAL_AND, $1, $3, @1); }
    | inner_expression "||" inner_expression { $$ = BIN_OP(BCC_AST_BINARY_OP_LOGICAL_OR, $1, $3, @1); }

assignment_expression
    : inner_expression
    | assignment_expression '='   assignment_expression { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_MAX); }
    | assignment_expression "+="  assignment_expression { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_PLUS); }
    | assignment_expression "-="  assignment_expression { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_MINUS); }
    | assignment_expression "*="  assignment_expression { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_MULT); }
    | assignment_expression "/="  assignment_expression { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_DIV); }
    | assignment_expression "%="  assignment_expression { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_MOD); }
    | assignment_expression ">>=" assignment_expression { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_SHIFTRIGHT); }
    | assignment_expression "<<=" assignment_expression { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_SHIFTLEFT); }
    | assignment_expression "&="  assignment_expression { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_BITWISE_AND); }
    | assignment_expression "|="  assignment_expression { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_BITWISE_OR); }
    | assignment_expression "^="  assignment_expression { $$ = ASSIGNMENT($1, $3, @1, BCC_AST_BINARY_OP_BITWISE_XOR); }

expression
    : assignment_expression

declaration_optional_assignment
    : type_declarator {
        struct bcc_ast_variable *var = create_bcc_ast_variable();

        var->type = $1.type;
        var->name = $1.ident;

        /* Fill in the declaration type */
        if (var->type) {
            struct bcc_ast_type *type;
            for (type = var->type; type->inner; type = type->inner)
                ;

            type->inner = state->declaration_type;
        } else {
            var->type = state->declaration_type;
        }

        if (var->type->node_type == BCC_AST_TYPE_PRIM && var->type->prim == BCC_AST_PRIM_VOID)
            ABORT_WITH_ERROR(&@1, "Variable cannot be of type void");

        char *str1 = bcc_ast_type_get_name(var->type);
        free(str1);

        list_add_tail(&state->current_scope->variable_list, &var->block_entry);
        list_add_tail(&state->current_func->local_variable_list, &var->func_entry);
    }
    | type_declarator '=' expression {
        struct bcc_ast_variable *var = create_bcc_ast_variable();

        var->type = $1.type;
        var->name = $1.ident;

        /* Fill in the declaration type */
        if (var->type) {
            struct bcc_ast_type *type;
            for (type = var->type; type->inner; type = type->inner)
                ;

            type->inner = state->declaration_type;
        } else {
            var->type = state->declaration_type;
        }

        if (var->type->node_type == BCC_AST_TYPE_PRIM && var->type->prim == BCC_AST_PRIM_VOID)
            ABORT_WITH_ERROR(&@1, "Variable cannot be of type void");

        list_add_tail(&state->current_scope->variable_list, &var->block_entry);
        list_add_tail(&state->current_func->local_variable_list, &var->func_entry);

        char *str1 = bcc_ast_type_get_name(var->type);
        free(str1);

        /* Create a new assignment of the expression to the just created variable */
        struct bae_var *store = create_bae_var(ast);
        store->var = var;
        store->ent.node_type = var->type;

        /* We temporarally remove 'const' from the variable to allow for initializing a const variable */
        bool const_is_set = flag_test(&var->type->qualifier_flags, BCC_AST_TYPE_QUALIFIER_CONST);
        flag_clear(&var->type->qualifier_flags, BCC_AST_TYPE_QUALIFIER_CONST);

        struct bcc_ast_entry *assign = create_assignment(ast, state, &store->ent, $3, BCC_AST_BINARY_OP_MAX);
        if (!assign)
            ABORT_WITH_ERROR(&@2, "Assignment not valid");

        if (const_is_set)
            flag_set(&var->type->qualifier_flags, BCC_AST_TYPE_QUALIFIER_CONST);

        struct bae_expression_stmt *stmt = create_bae_expression_stmt(ast);
        stmt->expression = assign;
        bae_block_add_entry(state->current_scope, &stmt->ent);
    }

declaration_list
    : declaration_optional_assignment
    | declaration_list ',' declaration_optional_assignment

declaration
    : type_base {
        state->declaration_type = $1;
        } declaration_list ';'


statement
    : expression ';' {
        struct bae_expression_stmt *stmt = create_bae_expression_stmt(ast);
        stmt->expression = $1;
        $$ = &stmt->ent;
    }
    | TOK_IF '(' expression ')' optional_block %prec "then" {
        struct bae_if *i = create_bae_if(ast);
        i->if_expression = $3;
        i->block = $5;
        $$ = &i->ent;
    }
    | TOK_IF '(' expression ')' optional_block TOK_ELSE optional_block {
        struct bae_if *i = create_bae_if(ast);
        i->if_expression = $3;
        i->block = $5;
        i->else_block = $7;
        $$ = &i->ent;
    }
    | TOK_WHILE '(' expression ')' optional_block {
        struct bae_while *w = create_bae_while(ast);
        w->condition = $3;
        w->block = $5;
        $$ = &w->ent;
    }
    | TOK_RETURN expression ';' {
        struct bae_return *r = create_bae_return(ast);
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
        struct bae_block *block = create_bae_block(ast);
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
    : type_name_optional_ident {
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
    : type_base TOK_IDENT '(' parameter_list_or_empty ')' {
        struct bae_function *func = create_bae_function(ast, $2);
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

static bool specifier_is_invalid(flags_t specifiers, int new_specifier)
{
    flags_t flags;

    if (flag_test(&specifiers, new_specifier))
        return true;

    switch (new_specifier) {
    case BCC_AST_TYPE_SPECIFIER_CHAR:
        flags = F(BCC_AST_TYPE_SPECIFIER_SIGNED) | F(BCC_AST_TYPE_SPECIFIER_UNSIGNED);
        return (specifiers & ~flags) != 0;

    case BCC_AST_TYPE_SPECIFIER_VOID:
        return specifiers != 0;

    case BCC_AST_TYPE_SPECIFIER_SHORT:
        flags = F(BCC_AST_TYPE_SPECIFIER_INT) | F(BCC_AST_TYPE_SPECIFIER_SIGNED) | F(BCC_AST_TYPE_SPECIFIER_UNSIGNED);
        return (specifiers & ~flags) != 0;

    case BCC_AST_TYPE_SPECIFIER_LONG:
        flags = F(BCC_AST_TYPE_SPECIFIER_INT) | F(BCC_AST_TYPE_SPECIFIER_SIGNED) | F(BCC_AST_TYPE_SPECIFIER_UNSIGNED);
        return (specifiers & ~flags) != 0;

    case BCC_AST_TYPE_SPECIFIER_INT:
        flags = F(BCC_AST_TYPE_SPECIFIER_SIGNED) | F(BCC_AST_TYPE_SPECIFIER_UNSIGNED) | F(BCC_AST_TYPE_SPECIFIER_LONG) | F(BCC_AST_TYPE_SPECIFIER_SHORT);
        return (specifiers & ~flags) != 0;

    case BCC_AST_TYPE_SPECIFIER_SIGNED:
    case BCC_AST_TYPE_SPECIFIER_UNSIGNED:
        flags = F(BCC_AST_TYPE_SPECIFIER_SIGNED) | F(BCC_AST_TYPE_SPECIFIER_UNSIGNED);
        return (specifiers & flags) != 0;
    }

    return false;
}

static void specifier_add_to_type(struct bcc_ast_type *type, int specifier)
{
    switch (specifier) {
    case BCC_AST_TYPE_SPECIFIER_VOID:
        type->prim = BCC_AST_PRIM_VOID;
        break;

    case BCC_AST_TYPE_SPECIFIER_INT:
        if (type->prim == BCC_AST_PRIM_MAX) {
            type->size = bcc_ast_type_primitives[BCC_AST_PRIM_INT].size;
            type->prim = BCC_AST_PRIM_INT;
        }
        break;

    case BCC_AST_TYPE_SPECIFIER_LONG:
        type->size = bcc_ast_type_primitives[BCC_AST_PRIM_LONG].size;
        type->prim = BCC_AST_PRIM_LONG;
        break;

    case BCC_AST_TYPE_SPECIFIER_SHORT:
        type->size = bcc_ast_type_primitives[BCC_AST_PRIM_SHORT].size;
        type->prim = BCC_AST_PRIM_SHORT;
        break;

    case BCC_AST_TYPE_SPECIFIER_CHAR:
        type->size = bcc_ast_type_primitives[BCC_AST_PRIM_CHAR].size;
        type->prim = BCC_AST_PRIM_CHAR;
        break;

    case BCC_AST_TYPE_SPECIFIER_UNSIGNED:
        type->is_unsigned = 1;
        if (type->prim == BCC_AST_PRIM_MAX) {
            type->size = bcc_ast_type_primitives[BCC_AST_PRIM_INT].size;
            type->prim = BCC_AST_PRIM_INT;
        }
        break;

    case BCC_AST_TYPE_SPECIFIER_SIGNED:
        if (type->prim == BCC_AST_PRIM_MAX) {
            type->size = bcc_ast_type_primitives[BCC_AST_PRIM_INT].size;
            type->prim = BCC_AST_PRIM_INT;
        }
        break;
    }
}

static struct bcc_ast_entry *create_bin_ptr_op(struct bcc_ast *ast, enum bcc_ast_binary_op op, struct bcc_ast_entry *ptr, struct bcc_ast_entry *val, int ptr_is_first)
{
    size_t ptr_size = ptr->node_type->inner->size;

    if (val->node_type->size < ptr_size) {
        struct bae_cast *cast = create_bae_cast(ast);
        cast->expr = val;
        cast->target = bcc_ast_type_primitives + BCC_AST_PRIM_LONG;
        cast->ent.node_type = cast->target;

        val = &cast->ent;
    }

    struct bae_binary_op *bin_op = create_bae_binary_op(ast, op);
    bin_op->operand_size = ptr_size;
    bin_op->ent.node_type = ptr->node_type;

    if (ptr_is_first) {
        bin_op->left = ptr;
        bin_op->right = val;
    } else {
        bin_op->left = val;
        bin_op->right = ptr;
    }

    return &bin_op->ent;
}

static struct bcc_ast_entry *create_ptr_bin_from_components(struct bcc_ast *ast, enum bcc_ast_binary_op op, struct bcc_ast_entry *left, struct bcc_ast_entry *right)
{
    struct bae_binary_op *bin_op;

    /* Verify that one side is a pointer, and one side is an integer */
    switch (op) {
    case BCC_AST_BINARY_OP_PLUS:
        if (bcc_ast_type_is_pointer(left->node_type) && bcc_ast_type_is_integer(right->node_type)) {
            return create_bin_ptr_op(ast, BCC_AST_BINARY_OP_ADDR_ADD_INDEX, left, right, 1);
        } else if (bcc_ast_type_is_pointer(right->node_type) && bcc_ast_type_is_integer(left->node_type)) {
            return create_bin_ptr_op(ast, BCC_AST_BINARY_OP_ADDR_ADD_INDEX, left, right, 0);
        } else {
            return NULL;
        }
        break;

    case BCC_AST_BINARY_OP_MINUS:
        if (bcc_ast_type_is_pointer(left->node_type) && bcc_ast_type_is_integer(right->node_type)) {
            return create_bin_ptr_op(ast, BCC_AST_BINARY_OP_ADDR_SUB_INDEX, left, right, 1);
        } else if (bcc_ast_type_is_pointer(right->node_type) && bcc_ast_type_is_integer(left->node_type)) {
            return create_bin_ptr_op(ast, BCC_AST_BINARY_OP_ADDR_SUB_INDEX, left, right, 0);
        } else if (bcc_ast_type_is_pointer(right->node_type) && bcc_ast_type_is_pointer(left->node_type)) {
            struct bae_binary_op *bin_op = create_bae_binary_op(ast, BCC_AST_BINARY_OP_ADDR_SUB);
            bin_op->operand_size = bae_size(right);
            bin_op->left = left;
            bin_op->right = right;
            bin_op->ent.node_type = bcc_ast_type_primitives + BCC_AST_PRIM_INT;
            return &bin_op->ent;
        } else {
            return NULL;
        }
        break;

    case BCC_AST_BINARY_OP_GREATER_THAN:
    case BCC_AST_BINARY_OP_GREATER_THAN_EQUAL:
    case BCC_AST_BINARY_OP_LESS_THAN:
    case BCC_AST_BINARY_OP_LESS_THAN_EQUAL:
    case BCC_AST_BINARY_OP_NOT_EQUAL:
    case BCC_AST_BINARY_OP_DOUBLEEQUAL:
        bin_op = create_bae_binary_op(ast, op);

        bin_op->left = left;
        bin_op->right = right;
        bin_op->ent.node_type = bcc_ast_type_primitives + BCC_AST_PRIM_INT;
        return &bin_op->ent;

    default:
        return NULL;
    }
}

static struct bcc_ast_entry *create_bin_from_components(struct bcc_ast *ast, enum bcc_ast_binary_op op, struct bcc_ast_entry *left, struct bcc_ast_entry *right)
{
    if (bcc_ast_type_is_pointer(left->node_type) || bcc_ast_type_is_pointer(right->node_type))
        return create_ptr_bin_from_components(ast, op, left, right);

    if (bcc_ast_type_is_integer(left->node_type) && bcc_ast_type_is_integer(right->node_type)) {
        size_t size_left = bae_size(left);
        size_t size_right = bae_size(right);

        /* Also check and handle unsigned casting */
        if (size_left < size_right) {
            struct bae_cast *cast = create_bae_cast(ast);
            cast->expr = left;
            cast->target = right->node_type;
            cast->ent.node_type = cast->target;

            left = &cast->ent;
        } else if (size_right < size_left) {
            struct bae_cast *cast = create_bae_cast(ast);
            cast->expr = right;
            cast->target = left->node_type;
            cast->ent.node_type = cast->target;

            right = &cast->ent;
        }
    }

    struct bae_binary_op *bin_op = create_bae_binary_op(ast, op);

    bin_op->left = left;
    bin_op->right = right;
    bin_op->ent.node_type = bin_op->left->node_type;
    return &bin_op->ent;
}

static struct bcc_ast_entry *create_assignment(struct bcc_ast *ast, struct bcc_parser_state *state, struct bcc_ast_entry *lvalue, struct bcc_ast_entry *rvalue, enum bcc_ast_binary_op bin_op)
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

