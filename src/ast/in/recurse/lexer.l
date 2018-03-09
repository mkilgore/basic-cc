%{
#include <string.h>
#include <stdio.h>

#include "ast.h"
#include "lexer.h"
#include "parser.h"

#define YY_USER_ACTION update_location(state, curtok);

static void update_location(struct bcc_lexer_state *state, struct lextok *);
static void state_append_str(struct bcc_lexer_state *state, char ch);

#define YY_DECL RECURSE_DECL

%}

%option prefix="recurse_"

%option noyywrap
%option noinput

%x string

%%

\r?\n {
    state->cur_tok.file_line_offset = state->file_offset;
    state->cur_tok.last_line++;
    state->cur_tok.last_column = 0;
}

[ \t]+ /* White space */

"!=" { curtok->tok = TOK_NOT_EQUAL; return ; }
">=" { curtok->tok = TOK_GREATER_THAN_EQUAL; return ; }
"<=" { curtok->tok = TOK_LESS_THAN_EQUAL; return ; }

"+" { curtok->tok = '+'; return ; }
"-" { curtok->tok = '-'; return ; }
"*" { curtok->tok = '*'; return ; }
"/" { curtok->tok = '/'; return ; }
"&" { curtok->tok = '&'; return ; }
"|" { curtok->tok = '|'; return ; }
"!" { curtok->tok = '!'; return ; }

"=" { curtok->tok = '='; return ; }
"<" { curtok->tok = '<'; return ; }
">" { curtok->tok = '>'; return ; }

"(" { curtok->tok = '('; return ; }
")" { curtok->tok = ')'; return ; }
"{" { curtok->tok = '{'; return ; }
"}" { curtok->tok = '}'; return ; }
";" { curtok->tok = ';'; return ; }
"," { curtok->tok = ','; return ; }

"\"" { BEGIN(string); }

[0-9]+ {
    curtok->tok = TOK_NUMBER;
    curtok->ival = atoi(yytext);
    return ;
}

"if"      { curtok->tok = TOK_IF; return ; }
"else"    { curtok->tok = TOK_ELSE; return ; }
"return"  { curtok->tok = TOK_RETURN; return ; }
"int"     { curtok->tok = TOK_TYPE; return ; }
"while"   { curtok->tok = TOK_WHILE; return ; }

[a-zA-Z_][a-zA-Z0-9_]* {
    curtok->str = strdup(yytext);

    /* Chek if it is a function ID */
    struct bae_function *func;
    list_foreach_entry(&ast->function_list, func, function_entry) {
        if (strcmp(func->name, curtok->str) == 0) {
            curtok->tok = TOK_FUNCTION;
            return ;
        }
    }

    curtok->tok = TOK_IDENT;
    return ;
}

<<EOF>> { curtok->tok = TOK_EOF; return ; }

<string>\" {
    state_append_str(state, '\0');
    curtok->str = state->lex_str;

    state->lex_str = NULL;
    state->lex_str_len = 0;
    state->lex_str_capacity = 0;

    BEGIN(INITIAL);
    curtok->tok = TOK_STRING;
    return ;
}

<string>\\[0-7]{1,3} {
    int result = strtol(yytext + 1, NULL, 8);
    state_append_str(state, (char)result);
}

<string>"\\x"[0-9a-fA-F]{1,2} {
    int result = strtol(yytext + 2, NULL, 16);
    state_append_str(state, (char)result);
}

<string>\\a { state_append_str(state, '\a'); }
<string>\\b { state_append_str(state, '\b'); }
<string>\\f { state_append_str(state, '\f'); }
<string>\\n { state_append_str(state, '\n'); }
<string>\\r { state_append_str(state, '\r'); }
<string>\\t { state_append_str(state, '\t'); }
<string>\\v { state_append_str(state, '\v'); }
<string>\\\\ { state_append_str(state, '\\'); }
<string>"\\'" { state_append_str(state, '\''); }
<string>"\\\"" { state_append_str(state, '\"'); }
<string>"\\?" { state_append_str(state, '\?'); }
<string>[^\\\"\n]+ {
    char *ptr = yytext;
    for (; *ptr; ptr++)
        state_append_str(state, *ptr);
}
%%

static void update_location(struct bcc_lexer_state *state, struct lextok *curtok)
{
    /* This works in conjunction with updates from the newline rule which
     * increment the current line */
    state->cur_tok.first_line = state->cur_tok.last_line;
    state->cur_tok.first_column = state->cur_tok.last_column;
    state->cur_tok.last_column += recurse_leng;
    state->file_offset += recurse_leng;

    curtok->location = state->cur_tok;

    /* Subtract one, because 'last_column' points to the first character of the next token
     * Not the last character of the previous token. */
    curtok->location.last_column--; 
}

static void state_append_str(struct bcc_lexer_state *state, char ch)
{
    state->lex_str_len++;
    if (state->lex_str_len >= state->lex_str_capacity) {
        state->lex_str_capacity *= 2;
        state->lex_str = realloc(state->lex_str, state->lex_str_capacity);
    }

    state->lex_str[state->lex_str_len - 1] = ch;
}

/* Get rid of error about unused yyunput */
void fake_func2(void)
{
    yyunput(0, 0);
}

