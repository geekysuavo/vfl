
/* include the main application header. */
#include "vflang.h"

/* application execution flags:
 *  @shall_persist: whether to become interactive after evaluations.
 *  @evals: string to evaluate after option parsing.
 */
static int shall_persist = 0;
static char *evals = NULL;

/* structures used for client/server communication:
 *  @is_daemon: whether to become a daemon. implies server.
 *  @hostname: hostname string for client/server modes.
 *  @client: client communication structure pointer.
 *  @server: server communication structure pointer.
 */
#ifdef __VFL_USE_LIBUV
static int is_daemon = 0;
static char *hostname = NULL;
static comm_client_t *client = NULL;
static comm_server_t *server = NULL;
#endif

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
   *  @opts: array of options recognized by getopt.
   *  @optcode: option value returned from getopt.
   *  @optidx: option array index from getopt.
   */
  int optcode, optidx;
  struct option opts[] = {
    /* argument-free options. */
    { "daemon",  no_argument, NULL, 'd' },
    { "persist", no_argument, NULL, 'p' },

    /* argument-associated options. */
    { "eval", required_argument, NULL, 'e' },
    { "host", required_argument, NULL, 'h' },

    /* end marker. */
    { NULL, 0, NULL, 0 }
  };

  /* loop until all options are handled. */
  while (1) {
    /* parse the next available option, or break. */
    optidx = 0;
    optcode = getopt_long(argc, argv, "dpe:h:", opts, &optidx);
    if (optcode == -1)
      break;

    /* handle the returned option code. */
    switch (optcode) {
      /* daemon flag. */
      case 'd':
#ifdef __VFL_USE_LIBUV
        is_daemon = 1;
        break;
#else
        fprintf(stderr, "%s: daemonization unsupported\n", argv[0]);
        return 1;
#endif

      /* persist flag. */
      case 'p':
        shall_persist = 1;
        break;

      /* eval argument. */
      case 'e':;
        /* resize the evaluation string. */
        const size_t n = (evals ? strlen(evals) : 0) + strlen(optarg);
        char *evnew = realloc(evals, n + 2);
        if (!evnew) {
          fprintf(stderr, "%s: failed to add evaluation\n", argv[0]);
          return 1;
        }

        /* concatenate the new string and break the case. */
        evals = evnew;
        strcat(evals, " ");
        strcat(evals, optarg);
        break;

      /* host argument. */
      case 'h':
#ifdef __VFL_USE_LIBUV
        /* attempt to set the hostname string. */
        hostname = strdup(optarg);
        if (!hostname) {
          fprintf(stderr, "%s: failed to set hostname\n", argv[0]);
          return 1;
        }

        /* break the case. */
        break;
#else
        fprintf(stderr, "%s: client/server mode unsupported\n", argv[0]);
        return 1;
#endif

      /* other. */
      case '?':
      default:
        break;
    }
  }

#ifdef __VFL_USE_LIBUV
  /* check if a hostname was supplied. */
  if (hostname) {
    /* check if daemonization was requested. */
    if (is_daemon) {
      /* construct a server on that hostname. */
      server = comm_server_alloc(hostname);
      if (!server) {
        fprintf(stderr, "%s: failed to serve on %s\n",
                argv[0], hostname);
        return 1;
      }
    }
    else {
      /* connect a client to that hostname. */
      client = comm_client_alloc(hostname);
      if (!client) {
        fprintf(stderr, "%s: failed to connect to %s\n",
                argv[0], hostname);
        return 1;
      }
    }

    /* free the hostname string. */
    free(hostname);
  }
  else if (is_daemon) {
    /* invalid configuration. */
    fprintf(stderr, "%s: cannot daemonize without hostname\n", argv[0]);
    return 1;
  }

  /* handle mutually exclusive flags. */
  if (server && shall_persist) {
    fprintf(stderr, "%s: cannot be an interactive daemon\n", argv[0]);
    return 1;
  }
#endif

  /* initialize the interpreter for non-client modes. */
#ifdef __VFL_USE_LIBUV
  if (!client) {
#endif
    if (!vfl_init()) {
      fprintf(stderr, "%s: failed to initialize interpreter\n", argv[0]);
      return 1;
    }
#ifdef __VFL_USE_LIBUV
  }
#endif

  /* handle the evaluation string. */
  if (evals) {
#ifdef __VFL_USE_LIBUV
    if (client && !comm_client_send(client, evals)) {
      fprintf(stderr, "%s: failed to execute string:\n"
              "  '%s'\n", argv[0], evals);
      return 1;
    }
    else
#endif
    if (!vfl_exec_string(evals)) {
      fprintf(stderr, "%s: failed to execute string:\n"
              "  '%s'\n", argv[0], evals);
      return 1;
    }
  }

  /* loop over any remaining (non-option) arguments. */
  for (int i = optind; i < argc; i++) {
#ifdef __VFL_USE_LIBUV
    /* attempt to execute the argument as a filename. */
    if (client && !comm_client_send_file(client, argv[i])) {
      fprintf(stderr, "%s: failed to execute '%s'\n", argv[0], argv[i]);
      return 1;
    }
    else
#endif
    if (!vfl_exec_path(argv[i])) {
      fprintf(stderr, "%s: failed to execute '%s'\n", argv[0], argv[i]);
      return 1;
    }
  }

  /* enter server or interactive mode. */
#ifdef __VFL_USE_LIBUV
  if (server) {
    /* FIXME: set up i/o and daemonize. */

    /* start running the server. */
    if (!comm_server_run(server))
      fprintf(stderr, "%s: unable to maintain server\n", argv[0]);
  }
  else
#endif
  if (shall_persist || (!evals && optind == argc)) {
    /* interactive mode. */
    unsigned int ncmd = 1;
    char prompt[33];

    /* loop until the session ends. */
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
#ifdef __VFL_USE_LIBUV
      if (client && !comm_client_send(client, pbuf)) {
        fprintf(stderr, "%s: failed to execute string\n", argv[0]);
        return 1;
      }
      else
#endif
      if (!vfl_exec_string(pbuf)) {
        fprintf(stderr, "%s: failed to execute string\n", argv[0]);
        return 1;
      }

      /* increment the command counter. */
      if (strcmp(pbuf, ""))
        ncmd++;
    }
  }

  /* free the client or server, if either is allocated. */
#ifdef __VFL_USE_LIBUV
  comm_client_free(client);
  comm_server_free(server);
#endif

  /* return success. */
  return 0;
}

