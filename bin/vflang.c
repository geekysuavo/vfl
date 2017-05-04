
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
   *  @prompt: prompt string buffer.
   *  @ncmd: command counter.
   */
  unsigned int ncmd = 1;
  char prompt[32];

  /* initialize the type registry. */
  if (!vfl_init()) {
    fprintf(stderr, "%s: failed to initialize type system\n", argv[0]);
    return 1;
  }

  /* determine the input method. */
  if (argc > 1) {
    /* file input. loop over each file. */
    for (int argi = 1; argi < argc; argi++) {
      /* execute the file. */
      if (!vfl_exec_path(argv[argi])) {
        fprintf(stderr, "%s: failed to execute '%s'\n", argv[0], argv[argi]);
        return 1;
      }
    }
  }
  else if (isatty(fileno(stdin))) {
    /* interactive. loop until the session ends. */
    while (1) {
      /* display a prompt. */
      snprintf(prompt, 32, "vfl:%u> ", ncmd);
      char *pbuf = readline(prompt);

      /* end the session if no input was read. */
      if (!pbuf)
        break;

      /* check for the quit command. */
      if (strncmp(pbuf, "quit", 4) == 0)
        break;

      /* execute the string. */
      add_history(pbuf);
      if (!vfl_exec_string(pbuf)) {
        fprintf(stderr, "%s: failed to execute string\n", argv[0]);
        return 1;
      }

      /* increment the command counter. */
      if (strcmp(pbuf, ""))
        ncmd++;
    }
  }
  else {
    /* execute from standard input. */
    if (!vfl_exec_file(stdin)) {
      fprintf(stderr, "%s: failed to execute stdin\n", argv[0]);
      return 1;
    }
  }

  /* return success. */
  return 0;
}

