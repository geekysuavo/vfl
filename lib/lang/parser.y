
/* enable verbose error messages. */
%error-verbose

%{
/* include c library headers. */
#include <stdio.h>

/* include vfl headers. */
#include <vfl/lang/object.h>

/* include the generated parser header. */
#include "lib/lang/parser.h"

/* scanner function declarations. */
void vfl_scanner_push_string (const char *str);
void vfl_scanner_push_file (FILE *fh);
void vfl_scanner_pop (void);

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

stmt
 : for
 | name T_SEMI
 | name T_EQUALS expr T_SEMI
 ;

expr
 : factor
 | expr T_PLUS factor
 | expr T_MINUS factor
 ;

factor
 : value
 | factor T_MUL value
 | factor T_DIV value
 ;

value
 : name
 | list
 | T_INT
 | T_FLOAT
 | T_STRING
 | T_PAREN_OPEN expr T_PAREN_CLOSE
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

/* vfl_exec_file(): interpret the contents of a file handle.
 *
 * arguments:
 *  @fh: file handle to interpret.
 *
 * returns:
 *  integer indicating execution success (1) or failure (0).
 */
int vfl_exec_file (FILE *fh) {
  /* check the input argument. */
  if (!fh)
    return 0;

  /* parse the file in a new buffer. */
  vfl_scanner_push_file(fh);
  int ret = (yyparse() == 0);

  /* release the buffer and return the result. */
  vfl_scanner_pop();
  return ret;
}

/* vfl_exec_path(): interpret the contents of a file, specified
 * using a filename.
 *
 * arguments:
 *  @path: path name of the file to interpret.
 *
 * returns:
 *  integer indicating execution success (1) or failure (0).
 */
int vfl_exec_path (const char *fname) {
  /* check the input argument. */
  if (!fname)
    return 0;

  /* open the file. */
  FILE *fh = fopen(fname, "r");
  if (!fh)
    return 0;

  /* parse the file in a new buffer. */
  vfl_scanner_push_file(fh);
  int ret = (yyparse() == 0);

  /* release the buffer and return the result. */
  vfl_scanner_pop();
  fclose(fh);
  return ret;
}

/* vfl_exec_string(): interpret the contents of a string.
 *
 * arguments:
 *  @str: string value to interpret.
 *
 * returns:
 *  integer indicating execution success (1) or failure (0).
 */
int vfl_exec_string (const char *str) {
  /* check the input argument. */
  if (!str)
    return 0;

  /* parse the string in a new buffer. */
  vfl_scanner_push_string(str);
  int ret = (yyparse() == 0);

  /* release the buffer and return the result. */
  vfl_scanner_pop();
  return ret;
}

