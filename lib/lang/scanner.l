
%{
/* include vfl headers. */
#include <vfl/lang/object.h>

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

