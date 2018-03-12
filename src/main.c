
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "arg_parser.h"

//static struct bcc_ast ast;
static const char *version_str = "basic-cc";
static const char *arg_str = "[Flags] [Input file]";

#define XARGS\
    X(output, "output", 1, 'o', "Define Output file (default: stdin)") \
    X(gen, "gen", 1, 'x', "Output format ('assembler', 'text', 'ast')") \
    X(help, "help", 0, 'h', "Display help") \
    X(version, "version", 0, 'v', "Display version information") \
    X(last, NULL, 0, '\0', NULL)

enum arg_index {
    ARG_EXTRA = ARG_PARSER_EXTRA,
    ARG_ERR = ARG_PARSER_ERR,
    ARG_DONE = ARG_PARSER_DONE,
#define X(enu, id, arg, op, help_text) ARG_##enu,
    XARGS
#undef X
};

static const struct arg bcc_args[] = {
#define X(enu, id, arg, op, help_text) [ARG_##enu] = { .lng = id, .shrt = op, .help_txt = help_text, .has_arg = arg },
    XARGS
#undef X
};

int main(int argc, char **argv)
{
    struct bcc_ast ast;
    FILE *in = stdin;
    FILE *out = stdout;
    enum arg_index ret;
    enum bcc_ast_out_format out_format = BCC_AST_OUT_ASM_X86;
    bcc_ast_init(&ast);

    while ((ret = arg_parser(argc, argv, bcc_args)) != ARG_DONE) {
        switch (ret) {
        case ARG_help:
            display_help_text(argv[0], arg_str, "", "", bcc_args);
            return 0;
        case ARG_version:
            printf("%s\n", version_str);
            return 0;

        case ARG_output:
            out = fopen(argarg, "w+");
            break;

        case ARG_gen:
            if (strcmp(argarg, "assembler") == 0)
                out_format = BCC_AST_OUT_ASM_X86;
            else if (strcmp(argarg, "text") == 0)
                out_format = BCC_AST_OUT_TEXT;
            else if (strcmp(argarg, "ast") == 0)
                out_format = BCC_AST_OUT_DUMP_AST;

            break;

        case ARG_EXTRA:
            in = fopen(argarg, "r");
            break;

        default:
            return 0;
        }
    }

    int parse_result = bcc_ast_parse(&ast, in);

    if (!parse_result)
        bcc_ast_out(&ast, out, out_format);

    if (in != stdin)
        fclose(in);

    if (out != stdout)
        fclose(out);

    bcc_ast_clear(&ast);

    return 0;
}

