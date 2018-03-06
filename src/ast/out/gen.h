#ifndef SRC_AST_OUT_GEN_h
#define SRC_AST_OUT_GEN_h

void gen_asm_x86(struct bcc_ast *ast, FILE *out);
void gen_text(struct bcc_ast *ast, FILE *out);
void gen_dump_ast(struct bcc_ast *ast, FILE *out);

#endif
