#ifndef INCLUDE_LEXTER_H
#define INCLUDE_LEXTER_H

#include <stdio.h>
#include "ast/ast.h"

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void *yyscan_t;
#endif

int yylex_init(yyscan_t *);
int yylex_destroy(yyscan_t);

void yyset_in(FILE *, yyscan_t scanner);
FILE *yyget_in(yyscan_t scanner);

#endif
