
/* enable verbose error messages. */
%error-verbose

%{
/* include c library headers. */
#include <stdio.h>

/* include vfl headers. */
#include <vfl/base/object.h>
#include <vfl/lang/ast.h>

/* include the generated parser header. */
#include "lib/lang/parser.h"

/* scanner function declarations. */
int vfl_parse_string (const char *str);
int vfl_parse_file (FILE *fh);

/* flex function declarations. */
void yyerror (const char *msg);
int yylex (void);

/* parsing results:
 *  @globals: the topmost symbol table.
 *  @tree: the abstract syntax tree root.
 */
static sym_table_t *globals = NULL;
static ast_t *tree = NULL;
%}

/* define the yylval type union. */
%union {
  /* literal values. */
  long v_int;
  double v_flt;
  char *v_str;

  /* tree nodes. */
  ast_t *v_ast;
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

/* declare the types of all tokens and symbols. */
%type <v_int> T_INT
%type <v_flt> T_FLOAT
%type <v_str> T_IDENT T_STRING
%type <v_ast> lang scope stmt_list stmt for
%type <v_ast> expr factor value name list int float string ident
%type <v_ast> expr_list arg_list qual_list arguments arg qual

%%

lang:        { /* empty. */ }
 | stmt_list { tree = $$ = $1; }
 ;

scope: T_BRACE_OPEN stmt_list T_BRACE_CLOSE { $$ = $2; };

stmt_list
 : stmt_list stmt { $$ = ast_list_append($1, $2); }
 | stmt           { $$ = ast_list(AST_NODE_BLOCK, $1); }
 ;

stmt
 : for
 | name T_SEMI
 | name T_EQUALS expr T_SEMI { $$ = ast_binary(AST_NODE_ASSIGN, $1, $3); }
 ;

expr
 : factor
 | expr T_PLUS factor  { $$ = ast_binary(AST_NODE_ADD, $1, $3); }
 | expr T_MINUS factor { $$ = ast_binary(AST_NODE_SUB, $1, $3); }
 ;

factor
 : value
 | factor T_MUL value { $$ = ast_binary(AST_NODE_MUL, $1, $3); }
 | factor T_DIV value { $$ = ast_binary(AST_NODE_DIV, $1, $3); }
 ;

value
 : name
 | list
 | int
 | float
 | string
 | T_PAREN_OPEN expr T_PAREN_CLOSE { $$ = $2; }
 ;

int: T_INT { $$ = ast_int($1); };

float: T_FLOAT { $$ = ast_float($1); };

string: T_STRING { $$ = ast_string(AST_NODE_STRING, $1); };

ident: T_IDENT { $$ = ast_string(AST_NODE_IDENT, $1); };

name
 : ident
 | ident qual_list { $$ = ast_binary(AST_NODE_NAME, $1, $2); }
 ;

qual_list
 : qual_list qual { $$ = ast_list_append($1, $2); }
 | qual           { $$ = ast_list(AST_NODE_QUALS, $1); }
 ;

qual
 : T_POINT ident { $$ = $2; }
 | arguments
 | list
 ;

arguments: T_PAREN_OPEN arg_list T_PAREN_CLOSE { $$ = $2; };

arg_list:               { $$ = ast_list(AST_NODE_ARGS, NULL); }
 | arg_list T_COMMA arg { $$ = ast_list_append($1, $3); }
 | arg                  { $$ = ast_list(AST_NODE_ARGS, $1); }
 ;

arg: ident T_COLON expr {
  $$ = ast_binary(AST_NODE_ARG, $1, $3);
};

list: T_BRACK_OPEN expr_list T_BRACK_CLOSE { $$ = $2 };

expr_list
 : expr_list T_COMMA expr { $$ = ast_list_append($1, $3); }
 | expr                   { $$ = ast_list(AST_NODE_LIST, $1); }
 ;

for: T_FOR ident T_IN value scope {
  $$ = ast_ternary(AST_NODE_FOR, $2, $4, $5);
};

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

/* vfl_prepare_parser(): prepare the global symbol table and
 * syntax tree for a new round of parsing.
 */
static void vfl_prepare_parser (void) {
  /* allocate the global symbol table. */
  if (!globals)
    globals = symbols_alloc(NULL);

  /* initialize the abstract syntax tree. */
  if (tree) {
    ast_free(tree);
    tree = NULL;
  }
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

  /* prepare the parser. */
  vfl_prepare_parser();

  /* parse the file. */
  if (!vfl_parse_file(fh))
    return 0;

  /* evaluate the resulting syntax tree. */
  if (!ast_eval(tree, globals))
    return 0;

  /* return success. */
  return 1;
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

  /* prepare the parser. */
  vfl_prepare_parser();

  /* open the file. */
  FILE *fh = fopen(fname, "r");
  if (!fh)
    return 0;

  /* parse and close the file. */
  const int ret = vfl_parse_file(fh);
  fclose(fh);

  /* check for parse failures. */
  if (!ret)
    return 0;

  /* evaluate the resulting syntax tree. */
  if (!ast_eval(tree, globals))
    return 0;

  /* return success. */
  return 1;
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

  /* prepare the parser. */
  vfl_prepare_parser();

  /* parse the string. */
  if (!vfl_parse_string(str))
    return 0;

  /* evaluate the resulting syntax tree. */
  if (!ast_eval(tree, globals))
    return 0;

  /* return success. */
  return 1;
}

