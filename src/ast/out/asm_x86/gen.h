#ifndef SRC_AST_OUT_ASM_X86_GEN_H
#define SRC_AST_OUT_ASM_X86_GEN_H

#include "../util.h"

struct gen_state {
    int next_label;
    FILE *out;
};

#define REG_ARG(id, siz) \
    (int)(id), (int)(siz)

void get_reg(char r, int size, char *id);
const char *gen_output_handle_escape(struct gen_state *state, const char *str, va_list *lst);
void gen_conv_op(struct gen_state *state, char r, int size_from, int size_to);

void gen_out(struct gen_state *state, const char *str, ...);

void gen_bcc_ast_entry(struct gen_state *, struct bcc_ast_entry *ent);
void gen_bcc_ast_store(struct gen_state *, struct bcc_ast_entry *store);

#endif
