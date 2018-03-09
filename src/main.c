
#include <stdio.h>
#include <string.h>
#include "ast.h"

static struct bcc_ast ast;

int main(int argc, char **argv)
{
    bcc_ast_init(&ast);

    if (argc < 2) {
        printf("Please provide filename to parse\n");
        printf("%s <filename> <output>\n", argv[0]);
        return 0;
    }

    FILE *in = fopen(argv[1], "r");
    FILE *out = stdout;

    if (argc == 3)
        out = fopen(argv[2], "w+");

    int ret = bcc_ast_parse(&ast, in);

    if (!ret) {
        //bcc_ast_out(&ast, out, BCC_AST_OUT_ASM_X86);
        //bcc_ast_out(&ast, out, BCC_AST_OUT_TEXT);
        bcc_ast_out(&ast, out, BCC_AST_OUT_DUMP_AST);
    }

    return 0;
}

