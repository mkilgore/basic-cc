%{
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "parser.tab.h"

#define YY_USER_ACTION update_location(yyscanner, state);

static void update_location(yyscan_t scan, struct bcc_parser_state *state);
static void state_append_str(struct bcc_parser_state *state, char ch);
%}

%option reentrant
%option bison-bridge
%option bison-locations

%option noyywrap
%option nounput
%option noinput
%option yylineno

%x string
%x char_literal

%%

\r?\n {
    yylloc->file_line_offset = state->file_offset;
    state->last_line++;
    state->last_column = 0;
}

[ \t]+ /* White space */

"if" { return TOK_IF; }
"else" { return TOK_ELSE; }
"return" { return TOK_RETURN; }
"while" { return TOK_WHILE; }

"int" { return TOK_INT; }
"long" { return TOK_LONG;}
"short" { return TOK_SHORT; }
"char" { return TOK_CHAR; }
"void" { return TOK_VOID; }

"++" { return TOK_PLUSPLUS; }
"--" { return TOK_MINUSMINUS; }

"<<" { return TOK_SHIFTLEFT; }
">>" { return TOK_SHIFTRIGHT; }

"&&" { return TOK_LOGICAL_AND; }
"||" { return TOK_LOGICAL_OR; }

"==" { return TOK_DOUBLE_EQUAL; }

"!=" { return TOK_NOT_EQUAL; }
">=" { return TOK_GREATER_THAN_EQUAL; }
"<=" { return TOK_LESS_THAN_EQUAL; }

"+=" { return TOK_ASSIGN_PLUS; }
"-=" { return TOK_ASSIGN_MINUS; }
"*=" { return TOK_ASSIGN_MULT; }
"/=" { return TOK_ASSIGN_DIV; }
"%=" { return TOK_ASSIGN_MOD; }
">>=" { return TOK_ASSIGN_SHIFTRIGHT; }
"<<=" { return TOK_ASSIGN_SHIFTLEFT; }
"&=" { return TOK_ASSIGN_AND; }
"|=" { return TOK_ASSIGN_OR; }
"^=" { return TOK_ASSIGN_XOR; }

"..." { return TOK_ELLIPSIS; }

"+" { return '+'; }
"-" { return '-'; }
"*" { return '*'; }
"/" { return '/'; }
"^" { return '^'; }
"%" { return '%'; }
"&" { return '&'; }
"|" { return '|'; }
"!" { return '!'; }
"~" { return '~'; }

"=" { return '='; }
"<" { return '<'; }
">" { return '>'; }

"(" { return '('; }
")" { return ')'; }
"{" { return '{'; }
"}" { return '}'; }
";" { return ';'; }
"," { return ','; }

"\"" {
    state->lexing_state = BCC_LEXER_STATE_STRING_LITERAL;
    BEGIN(string);
}

' {
    state->lex_int = 0;
    state->lexing_state = BCC_LEXER_STATE_CHAR_LITERAL;
    BEGIN(char_literal);
}


[0-9]+ {
    yylval->ival = atoi(yytext);
    return TOK_NUMBER;
}

0[xX][0-9a-fA-F]+ {
    yylval->ival = strtol(yytext + 2, 0, 16);
    return TOK_NUMBER;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->str = strdup(yytext);

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

<string>\n {
    parser_warning(yylloc, yyscanner, "Unexpected newline in string constant, ignored");
    yylloc->file_line_offset = state->file_offset;
    state->last_line++;
    state->last_column = 0;
}

<string,char_literal>\\[0-7]{1,3} {
    int result = strtol(yytext + 1, NULL, 8);
    state_append_str(state, (char)result);
}

<string,char_literal>"\\x"[0-9a-fA-F]{1,2} {
    int result = strtol(yytext + 2, NULL, 16);
    state_append_str(state, (char)result);
}

<string,char_literal>\\a { state_append_str(state, '\a'); }
<string,char_literal>\\b { state_append_str(state, '\b'); }
<string,char_literal>\\f { state_append_str(state, '\f'); }
<string,char_literal>\\n { state_append_str(state, '\n'); }
<string,char_literal>\\r { state_append_str(state, '\r'); }
<string,char_literal>\\t { state_append_str(state, '\t'); }
<string,char_literal>\\v { state_append_str(state, '\v'); }
<string,char_literal>\\\\ { state_append_str(state, '\\'); }
<string,char_literal>"\\'" { state_append_str(state, '\''); }
<string,char_literal>"\\\"" { state_append_str(state, '\"'); }
<string,char_literal>"\\?" { state_append_str(state, '\?'); }
<string>[^\\\"\n]+ {
    char *ptr = yytext;
    for (; *ptr; ptr++)
        state_append_str(state, *ptr);
}

<char_literal>[^\\\'\n]+ {
    char *ptr = yytext;
    for (; *ptr; ptr++)
        state_append_str(state, *ptr);
}

<char_literal>' {
    yylval->ival = state->lex_int;
    BEGIN(INITIAL);
    return TOK_NUMBER;
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
    unsigned int temp = ch;

    switch (state->lexing_state) {
    case BCC_LEXER_STATE_CHAR_LITERAL:
        state->lex_int <<= 8;
        state->lex_int |= temp;
        break;

    case BCC_LEXER_STATE_STRING_LITERAL:
        state->lex_str_len++;
        if (state->lex_str_len >= state->lex_str_capacity) {
            if (state->lex_str_capacity == 0)
                state->lex_str_capacity = 20;
            else
                state->lex_str_capacity *= 2;

            state->lex_str = realloc(state->lex_str, state->lex_str_capacity);
        }

        state->lex_str[state->lex_str_len - 1] = ch;
        break;
    }
}

