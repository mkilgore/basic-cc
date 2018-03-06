#ifndef INCLUDE_LEXTER_H
#define INCLUDE_LEXTER_H

#include <stdio.h>
#include "ast/ast.h"
#include "parser.h"

int yylex(struct bcc_ast *, struct bcc_parser_state *);
int yylex_destroy(void);

extern FILE *yyin;


#endif
