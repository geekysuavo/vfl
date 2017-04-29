
/* enable verbose error messages. */
%error-verbose

%{
/* include c library headers. */
#include <stdio.h>

/* include vfl headers. */
#include <vfl/lang/object.h>

/* include the generated parser header. */
#include "lib/lang/parser.h"

/* flex function declarations. */
void yyerror (const char *msg);
int yylex (void);
%}

/* define the yylval type union. */
%union {
  long v_int;
  double v_flt;
  char *v_str;
}

/* define all recognized tokens. */
%token T_INT T_FLOAT T_STRING T_IDENT
%token T_PAREN_OPEN T_PAREN_CLOSE T_BRACK_OPEN T_BRACK_CLOSE
%token T_EQUALS T_PLUS T_MINUS T_MUL T_DIV T_COMMA T_COLON T_SEMI
%token T_UNKNOWN

%%

lang:;

%%

/* yyerror(): output a parser error message.
 *
 * arguments:
 *  @msg: error message string.
 */
void yyerror (const char *msg) {
  /* for now, output directly to standard error. */
  fprintf(stderr, msg);
  fflush(stderr);
}

