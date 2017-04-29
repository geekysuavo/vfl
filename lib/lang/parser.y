
/* enable verbose error messages. */
%error-verbose

%{
/* include c library headers. */
#include <stdio.h>

/* include vfl headers. */
#include <vfl/lang/object.h>

/* include the generated parser header. */
#include "lib/lang/parser.h"

/* flex variable and function declarations. */
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
%token T_PAREN_OPEN T_PAREN_CLOSE
%token T_BRACK_OPEN T_BRACK_CLOSE
%token T_BRACE_OPEN T_BRACE_CLOSE
%token T_EQUALS T_PLUS T_MINUS T_MUL T_DIV
%token T_POINT T_COMMA T_COLON T_SEMI
%token T_FOR T_IN
%token T_UNKNOWN

%%

lang:
 | stmt_list;

scope: T_BRACE_OPEN stmt_list T_BRACE_CLOSE;

stmt_list: stmt_list stmt | stmt;

stmt: for | expr T_SEMI;

expr: value | name T_EQUALS value;

value
 : name
 | list
 | T_INT
 | T_FLOAT
 | T_STRING
 ;

name: T_IDENT | T_IDENT qual_list;

qual_list: qual_list qual | qual;

qual
 : T_POINT T_IDENT
 | T_PAREN_OPEN arg_list T_PAREN_CLOSE
 | list
 ;

arg_list:
 | arg_list T_COMMA arg
 | arg
 ;

arg: T_IDENT T_COLON value;

list: T_BRACK_OPEN expr_list T_BRACK_CLOSE;

expr_list: expr_list T_COMMA expr | expr;

for: T_FOR T_IDENT T_IN value scope;

%%

/* yyerror(): output a parser error message.
 *
 * arguments:
 *  @msg: error message string.
 */
void yyerror (const char *msg) {
  /* for now, output directly to standard error. */
  fprintf(stderr, "%s\n", msg);
  fflush(stderr);
}

