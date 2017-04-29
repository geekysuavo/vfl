
/* include vfl headers. */
#include <vfl/vfl.h>

/* include headers for command input. */
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

/* main(): application entry point.
 *
 * arguments:
 *  @argc: number of command-line arguments.
 *  @argv: array of command-line arguments.
 *
 * returns:
 *  application exit status.
 */
int main (int argc, char **argv) {
  /* declare required variables:
   *  @buf: buffer string for reading input.
   */
  char buf[1024];

  /* initialize the type registry. */
  if (!vfl_init()) {
    fprintf(stderr, "%s: failed to initialize type system\n", argv[0]);
    return 1;
  }

  /* FIXME: determine the input method. */
  printf("yyparse() => %d\n", vflang_parse());

  /* return success. */
  return 0;
}

