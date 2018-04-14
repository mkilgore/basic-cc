#ifndef SRC_AST_IN_PARSER_H
#define SRC_AST_IN_PARSER_H

#include "ast/ast.h"
#include "lexer.h"

struct bcc_parser_state {
    struct bae_block *current_scope;
    struct bae_function *current_func;

    struct bcc_ast_type *declaration_type;

    enum lexing_state {
        BCC_LEXER_STATE_STRING_LITERAL,
        BCC_LEXER_STATE_CHAR_LITERAL,
    } lexing_state;

    char *lex_str;
    unsigned int lex_int;
    size_t lex_str_capacity;
    size_t lex_str_len;

    int first_line;
    int first_column;
    int last_line;
    int last_column;
    off_t file_offset;

    yyscan_t scanner;
    struct bcc_ast *ast;
};

#define BCC_PARSER_STATE_INIT(e) \
    { \
    }

static inline void bcc_parser_state_init(struct bcc_parser_state *state)
{
    *state = (struct bcc_parser_state)BCC_PARSER_STATE_INIT(*state);
}

int yyparse(struct bcc_ast *, struct bcc_parser_state *, yyscan_t);

#endif
