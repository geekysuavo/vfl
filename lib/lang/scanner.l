
%{
/* include vfl headers. */
#include <vfl/lang/object.h>
#include <vfl/lang/ast.h>

/* include the generated parser header. */
#include "lib/lang/parser.h"
%}

%option noyywrap
%option noinput
%option nounput
%%

'[^']+' {
  /* allocate memory for the string data. */
  size_t n = strlen(yytext);
  yylval.v_str = malloc(n);

  /* if possible, store the string data. */
  if (yylval.v_str) {
    strncpy(yylval.v_str, yytext + 1, n - 2);
    yylval.v_str[n - 2] = '\0';
  }

  /* return the token. */
  return T_STRING;
}

[-+]?[0-9]+ {
  /* parse the integer value and return the token. */
  yylval.v_int = atol(yytext);
  return T_INT;
}

[-+]?([0-9]*\.?[0-9]+|[0-9]+\.)([eE][-+]?[0-9]+)? {
  /* parse the float value and return the token. */
  yylval.v_flt = atof(yytext);
  return T_FLOAT;
}

for { return T_FOR; }
in  { return T_IN; }

[a-zA-Z][a-zA-Z0-9_]* {
  /* allocate memory for the identifier value. */
  size_t n = strlen(yytext);
  yylval.v_str = malloc(n + 1);

  /* if possible, store the identifier value. */
  if (yylval.v_str) {
    strcpy(yylval.v_str, yytext);
    yylval.v_str[n] = '\0';
  }

  /* return the token. */
  return T_IDENT;
}

#.* {}
[ \t\n\r] {}

"(" { return T_PAREN_OPEN; }
")" { return T_PAREN_CLOSE; }
"[" { return T_BRACK_OPEN; }
"]" { return T_BRACK_CLOSE; }
"{" { return T_BRACE_OPEN; }
"}" { return T_BRACE_CLOSE; }

"=" { return T_EQUALS; }
"+" { return T_PLUS; }
"-" { return T_MINUS; }
"*" { return T_MUL; }
"/" { return T_DIV; }

"." { return T_POINT; }
"," { return T_COMMA; }
":" { return T_COLON; }
";" { return T_SEMI; }

. { return T_UNKNOWN; }

%%

/* vfl_scanner_push_file(): push a new buffer state associated with
 * a file handle onto the buffer stack managed by flex.
 *
 * arguments:
 *  @fh: file handle for the new buffer.
 */
void vfl_scanner_push_file (FILE *fh) {
  /* push a file-based buffer to the stack. */
  yypush_buffer_state(yy_create_buffer(fh, YY_BUF_SIZE));
}

/* vfl_scanner_push_string(): push a new buffer state associated with
 * a string onto the buffer stack managed by flex.
 *
 * arguments:
 *  @str: string value for the new buffer.
 */
void vfl_scanner_push_string (const char *str) {
  /* push a string-based buffer to the stack. */
  yypush_buffer_state(yy_scan_string(str));
}

/* vfl_scanner_pop(): pop the topmost buffer state from the buffer
 * stack managed by flex.
 */
void vfl_scanner_pop (void) {
  /* pop the current buffer from the stack. */
  yypop_buffer_state();
}

