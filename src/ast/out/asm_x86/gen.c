
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "ast.h"
#include "gen.h"

/* id must be length 4 */
void get_reg(char r, int size, char *id)
{
    switch (size) {
    case 1:
        id[0] = r;
        id[1] = 'l';
        id[2] = '\0';
        return ;

    case 2:
        id[0] = r;
        id[1] = 'x';
        id[2] = '\0';
        return ;

    case 4:
        id[0] = 'e';
        id[1] = r;
        id[2] = 'x';
        id[3] = '\0';
        return ;

    default:
        id[0] = '\0';
        return ;
    }
}

const char *gen_output_handle_escape(struct gen_state *state, const char *str, va_list *lst)
{
    char reg_id[4];
    char r;
    int s;
    const char *arg_str;

    str++;
    switch (*str) {
    case '%':
        fputc(*str, state->out);
        break;

    case 'r':
        r = va_arg(*lst, int);
        s = va_arg(*lst, int);

        get_reg(r, s, reg_id);

        fputc('%', state->out);
        fputs(reg_id, state->out);
        break;

    case 'd':
        s = va_arg(*lst, int);
        fprintf(state->out, "%d", s);
        break;

    case 'c':
        r = va_arg(*lst, int);
        fputc(r, state->out);
        break;

    case 's':
        arg_str = va_arg(*lst, const char *);
        fprintf(state->out, "%s", arg_str);
        break;

    default:
        printf("Warning: Incorrect escape character\n");
        break;
    }

    return str;
}

/*
 * Takes a custom format string and writes the formatted text to the output
 */
void gen_out(struct gen_state *state, const char *str, ...)
{
    va_list lst;
    va_start(lst, str);

    for (; *str; str++) {
        switch (*str) {
        case '%':
            str = gen_output_handle_escape(state, str, &lst);
            break;

        default:
            fputc(*str, state->out);
            break;
        }
    }

    va_end(lst);
}

void gen_conv_op(struct gen_state *state, char r, int size_from, int size_to, int is_unsigned)
{
    switch (size_from) {
    case 1:
        switch (size_to) {
        case 1:
            break;

        case 2:
            gen_out(state, "    mov%cbw %r, %r\n", (is_unsigned)? 'z': 's', REG_ARG(r, 1), REG_ARG(r, 2));
            break;

        case 4:
            gen_out(state, "    mov%cbl %r, %r\n", (is_unsigned)? 'z': 's', REG_ARG(r, 1), REG_ARG(r, 4));
            break;
        }
        break;

    case 2:
        switch (size_to) {
        case 1:
        case 2:
            break;

        case 4:
            gen_out(state, "    movswl %r, %r\n", REG_ARG(r, 2), REG_ARG(r, 4));
            break;
        }
        break;

    case 4:
        break;
    }
}

static void gen_bcc_ast_function(struct gen_state *state, struct bae_function *func, FILE *out)
{
    struct bcc_ast_variable *var;
    int local_var_count = 0;

    if (func->block) {
        gen_out(state, ".global %s\n", func->name);
        gen_out(state, "%s:\n",  func->name);
        gen_out(state, "    pushl %%ebp\n");
        gen_out(state, "    movl %%esp, %%ebp\n");

        list_foreach_entry_reverse(&func->local_variable_list, var, func_entry) {
            local_var_count++;
            var->loffset = local_var_count * 4;
        }

        gen_out(state, "    subl $%d, %%esp\n", local_var_count * 4);

        gen_bcc_ast_entry(state, func->block);
    } else {
        gen_out(state, ".extern %s\n", func->name);
    }
}

static void gen_bcc_literal_strings(struct gen_state *state, struct bcc_ast *ast)
{
    struct bae_literal_string *lit_str;
    gen_out(state, ".section .rodata\n");

    list_foreach_entry(&ast->literal_string_list, lit_str, literal_string_entry) {
        char *esc = util_escape_str(lit_str->str);

        gen_out(state, ".LS%d: .asciz \"%s\"\n", lit_str->string_id, esc);
        free(esc);
    }
}

void gen_asm_x86(struct bcc_ast *ast, FILE *out)
{
    struct gen_state state;
    struct bae_function *func;

    memset(&state, 0, sizeof(state));

    state.out = out;

    gen_bcc_literal_strings(&state, ast);

    gen_out(&state, ".text\n");

    list_foreach_entry(&ast->function_list, func, function_entry)
        gen_bcc_ast_function(&state, func, out);
}

