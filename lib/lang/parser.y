
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

/* flex function declarations. */
void yyerror (const char *msg);
int yylex (void);

/* vfl function declarations. */
void vfl_set_tree (ast_t *node);
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
%token T_EQUALS T_PLUS T_MINUS T_MUL T_DIV T_POW
%token T_EQ T_NE T_LT T_GT T_LE T_GE
%token T_POINT T_COMMA T_COLON T_SEMI
%token T_IMPORT T_IF T_ELIF T_ELSE T_FOR T_IN T_WHILE
%token T_UNKNOWN

/* declare the types of all tokens and symbols. */
%type <v_int> T_INT
%type <v_flt> T_FLOAT
%type <v_str> T_IDENT T_STRING
%type <v_ast> lang scope stmt_list stmt
%type <v_ast> if elifs if_blk elif_blk else_blk for while import
%type <v_ast> expr cmp term factor value name list int float string ident
%type <v_ast> expr_list arg_list qual_list arguments arg qual

%%

lang:        { /* empty. */ }
 | stmt_list { $$ = $1; vfl_set_tree($1); }
 ;

scope: T_BRACE_OPEN stmt_list T_BRACE_CLOSE { $$ = $2; };

stmt_list
 : stmt_list stmt { $$ = ast_list_append($1, $2); }
 | stmt           { $$ = ast_list(AST_NODE_BLOCK, $1); }
 ;

stmt
 : if
 | for
 | while
 | import T_SEMI
 | name T_SEMI
 | name T_EQUALS expr T_SEMI { $$ = ast_binary(AST_NODE_ASSIGN, $1, $3); }
 ;

expr
 : cmp
 | expr T_EQ cmp { $$ = ast_binary(AST_NODE_EQ, $1, $3); }
 | expr T_NE cmp { $$ = ast_binary(AST_NODE_NE, $1, $3); }
 | expr T_LT cmp { $$ = ast_binary(AST_NODE_LT, $1, $3); }
 | expr T_GT cmp { $$ = ast_binary(AST_NODE_GT, $1, $3); }
 | expr T_LE cmp { $$ = ast_binary(AST_NODE_LE, $1, $3); }
 | expr T_GE cmp { $$ = ast_binary(AST_NODE_GE, $1, $3); }
 ;

cmp
 : term
 | cmp T_PLUS term  { $$ = ast_binary(AST_NODE_ADD, $1, $3); }
 | cmp T_MINUS term { $$ = ast_binary(AST_NODE_SUB, $1, $3); }
 ;

term
 : factor
 | term T_MUL factor { $$ = ast_binary(AST_NODE_MUL, $1, $3); }
 | term T_DIV factor { $$ = ast_binary(AST_NODE_DIV, $1, $3); }
 ;

factor
 : value
 | factor T_POW value { $$ = ast_binary(AST_NODE_POW, $1, $3); }
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

list: T_BRACK_OPEN expr_list T_BRACK_CLOSE { $$ = $2; };

expr_list
 : expr_list T_COMMA expr { $$ = ast_list_append($1, $3); }
 | expr                   { $$ = ast_list(AST_NODE_LIST, $1); }
 ;

if: elifs
 | elifs else_blk { $$ = ast_list_append($1, $2); }
 ;

elifs
 : if_blk         { $$ = ast_list(AST_NODE_IFS, $1); }
 | elifs elif_blk { $$ = ast_list_append($1, $2); }
 ;

if_blk: T_IF expr scope { $$ = ast_binary(AST_NODE_IF, $2, $3); };

elif_blk: T_ELIF expr scope { $$ = ast_binary(AST_NODE_IF, $2, $3); };

else_blk: T_ELSE scope { $$ = $2; };

for: T_FOR ident T_IN value scope {
  $$ = ast_ternary(AST_NODE_FOR, $2, $4, $5);
};

while: T_WHILE expr scope {
  $$ = ast_binary(AST_NODE_WHILE, $2, $3);
}

import: T_IMPORT ident {
  ast_node_type($2) = AST_NODE_IMPORT;
  $$ = $2;
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

