%{
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "parser.tab.h"

#define YY_NO_INPUT

#define YY_USER_ACTION update_location(yyscanner, state);

static void update_location(yyscan_t scan, struct bcc_parser_state *state);
static void state_append_str(struct bcc_parser_state *state, char ch);
%}

%option reentrant
%option bison-bridge
%option bison-locations

%option noyywrap
%option nounput
%option yylineno

%x string

%%

\r?\n {
    yylloc->file_line_offset = state->file_offset;
    state->last_line++;
    state->last_column = 0;
}

[ \t]+ /* White space */

"!=" { return TOK_NOT_EQUAL; }
">=" { return TOK_GREATER_THAN_EQUAL; }
"<=" { return TOK_LESS_THAN_EQUAL; }

"+" { return '+'; }
"-" { return '-'; }
"*" { return '*'; }
"/" { return '/'; }
"&" { return '&'; }
"|" { return '|'; }
"!" { return '!'; }

"=" { return '='; }
"<" { return '<'; }
">" { return '>'; }

"(" { return '('; }
")" { return ')'; }
"{" { return '{'; }
"}" { return '}'; }
";" { return ';'; }
"," { return ','; }

"\"" { BEGIN(string); }

[0-9]+ {
    yylval->ival = atoi(yytext);
    return TOK_NUMBER;
}

"if" { return TOK_IF; }
"else" { return TOK_ELSE; }
"return" { return TOK_RETURN; }
"int" { return TOK_TYPE; }
"while" { return TOK_WHILE; }

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->str = strdup(yytext);

    /* Chek if it is a function ID */
    struct bae_function *func;
    list_foreach_entry(&ast->function_list, func, function_entry)
        if (strcmp(func->name, yylval->str) == 0)
            return TOK_FUNCTION;

    return TOK_IDENT;
}

<<EOF>> { return TOK_EOF; }

<string>\" {
    state_append_str(state, '\0');
    yylval->str = state->lex_str;

    state->lex_str = NULL;
    state->lex_str_len = 0;
    state->lex_str_capacity = 0;

    BEGIN(INITIAL);
    return TOK_STRING;
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

static void update_location(yyscan_t scan, struct bcc_parser_state *state)
{
    /* This works in conjunction with updates from the newline rule which
     * increment the current line */
    state->first_line = state->last_line;
    state->first_column = state->last_column;
    state->last_column += yyget_leng(scan);
    state->file_offset += yyget_leng(scan);

    YYLTYPE *lloc = yyget_lloc(scan);

    lloc->first_line = state->first_line;
    lloc->last_line = state->last_line;
    lloc->first_column = state->first_column;

    /* Subtract one, because 'last_column' points to the first character of the next token
     * Not the last character of the previous token. */
    lloc->last_column = state->last_column - 1;
}

static void state_append_str(struct bcc_parser_state *state, char ch)
{
    state->lex_str_len++;
    if (state->lex_str_len >= state->lex_str_capacity) {
        state->lex_str_capacity *= 2;
        state->lex_str = realloc(state->lex_str, state->lex_str_capacity);
    }

    state->lex_str[state->lex_str_len - 1] = ch;
}

