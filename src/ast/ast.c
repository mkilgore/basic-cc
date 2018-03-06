
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "container_of.h"

#include "ast.h"
#include "in/parser.h"
#include "in/lexer.h"
#include "out/gen.h"

int bcc_ast_parse(struct bcc_ast *ast, FILE *in)
{
    struct bcc_parser_state state;
    bcc_parser_state_init(&state);

    yyin = in;

    int ret = yyparse(ast, &state);

    yyin = NULL;

    return ret;
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

