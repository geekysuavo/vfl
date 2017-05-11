
/* include the main application header. */
#include "vflang.h"

/* application execution flags:
 *  @shall_persist: whether or not to force interactivity.
 *  @evals: string to evaluate after option parsing.
 */
static int shall_persist = 0;
static char *evals = NULL;

#ifdef __VFL_USE_LIBUV
/* variables used for client/server communication:
 *  @is_server: whether or not the hostname is used for a server.
 *  @is_daemon: whether or not to disappear into the background.
 *  @client: client communication structure pointer.
 *  @server: server communication structure pointer.
 *  @hostname: hostname string for client/server modes.
 *  @pipefd: file descriptors for buffering stdout.
 */
static int is_server = 0, is_daemon = 0;
static comm_client_t *client = NULL;
static comm_server_t *server = NULL;
static char *hostname = NULL;
static int pipefd[2];

/* N_BLK: number of bytes per block to read from the pipe. */
#define N_BLK 256

/* cb_server(): callback function for returning
 * interpreter outputs back to clients.
 *  - see comm_server_cb() for details.
 */
static int cb_server (const char *msg, uv_buf_t *buf) {
  /* send the message string to the interpreter. */
  if (!vfl_exec_string(msg))
    fprintf(stderr, "error:\n\n>>  %s\n", msg);
  else
    comm_reply_set_status(buf, 1);

  /* flush standard output to the pipe. */
  printf("\n");
  fflush(stdout);

  /* collect the interpreter output from the pipe. */
  char blk[N_BLK + 1];
  ssize_t n = 0;
  do {
    /* initialize the data block to ensure nul-termination. */
    memset(blk, 0, N_BLK + 1);

    /* read a block of data from the pipe. */
    n = read(pipefd[0], &blk, N_BLK);
    if (n <= 0)
      break;

    /* append the data to the response buffer. */
    comm_reply_append_str(buf, blk);
  }
  while (n == N_BLK);

  /* do not halt the server. */
  return 0;
}
#endif

/* do_string(): execute a string, either locally or through the network.
 *
 * arguments:
 *  @str: string to execute.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
static int do_string (const char *str) {
#ifdef __VFL_USE_LIBUV
  /* when a network client is available, use it. */
  if (client)
    return comm_client_send(client, str);
#endif

  /* attempt to locally execute the string. */
  return vfl_exec_string(str);
}

/* do_file(): execute a file, either locally or through the network.
 *
 * arguments:
 *  @fname: filename to execute.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
static int do_file (const char *fname) {
#ifdef __VFL_USE_LIBUV
  /* when a network client is available, use it. */
  if (client)
    return comm_client_send_file(client, fname);
#endif

  /* attempt to locally execute the file. */
  return vfl_exec_path(fname);
}

/* --- */

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
   *  @status: status code to return on exit.
   */
  int optcode, optidx, status = EXIT_FAILURE;
  struct option opts[] = {
    /* argument-free options. */
    { "daemon",  no_argument, NULL, 'd' },
    { "server",  no_argument, NULL, 's' },
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
    optcode = getopt_long(argc, argv, "dspe:h:", opts, &optidx);
    if (optcode == -1)
      break;

    /* handle the returned option code. */
    switch (optcode) {
#ifdef __VFL_USE_LIBUV
      /* daemon and server flags. */
      case 'd': is_daemon = 1;
      case 's': is_server = 1; break;

      /* host argument. */
      case 'h':
        /* attempt to set the hostname string. */
        hostname = strdup(optarg);
        if (hostname) break;
        fprintf(stderr, "%s: failed to set hostname\n", argv[0]);
        goto fail;
#else
      /* daemon, server, and hostname flags. */
      case 'd':
      case 's':
      case 'h':
        fprintf(stderr, "%s: network operation is unsupported\n", argv[0]);
        goto fail;
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

      /* other. */
      case '?':
      default:
        break;
    }
  }

#ifdef __VFL_USE_LIBUV
  /* handle mutually exclusive flags. */
  if (is_server && (shall_persist || !hostname)) {
    fprintf(stderr, "%s: arguments are mutually exclusive\n", argv[0]);
    goto fail;
  }

  /* handle server output redirection. */
  if (is_server) {
    /* create a pipe for redirecting standard output. */
    if (pipe(pipefd) != 0) {
      fprintf(stderr, "%s: failed to redirect server output\n", argv[0]);
      goto fail;
    }

    /* redirect stdout to the pipe. */
    close(STDOUT_FILENO);
    dup(pipefd[1]);
  }

  /* handle daemonization. */
  if (is_daemon) {
    /* fork a child process. */
    pid_t newpid = fork();
    if (newpid < 0) {
      fprintf(stderr, "%s: failed to fork for daemon\n", argv[0]);
      goto fail;
    }

    /* kill the parent process. */
    if (newpid > 0)
      exit(EXIT_SUCCESS);

    /* reset the file mask and change directory to the root. */
    umask(0);
    chdir("/");

    /* create a new session. */
    pid_t sess = setsid();
    if (sess < 0)
      goto fail;

    /* close down standard input. */
    close(STDIN_FILENO);

    /* wait a bit before running the server. */
    sleep(1);
  }

  /* check if a hostname was supplied for network operation. */
  if (hostname) {
    /* check if a client or server is required. */
    if (is_server) {
      /* construct a server on that hostname. */
      server = comm_server_alloc(hostname, cb_server);
      if (!server) {
        fprintf(stderr, "%s: failed to start server on %s\n",
                argv[0], hostname);
        goto fail;
      }
    }
    else {
      /* connect a client to that hostname. */
      client = comm_client_alloc(hostname);
      if (!client) {
        fprintf(stderr, "%s: failed to connect to %s\n",
                argv[0], hostname);
        goto fail;
      }
    }
  }
#endif

  /* initialize the interpreter for non-client modes. */
#ifdef __VFL_USE_LIBUV
  if (!client) {
#endif
    if (!vfl_init()) {
      fprintf(stderr, "%s: failed to initialize interpreter\n", argv[0]);
      goto fail;
    }
#ifdef __VFL_USE_LIBUV
  }
#endif

  /* handle the evaluation string. */
  if (evals && !do_string(evals)) {
    fprintf(stderr, "%s: failed to execute:\n>> %s\n", argv[0], evals);
    goto fail;
  }

  /* loop over any remaining (non-option) arguments. */
  for (int i = optind; i < argc; i++) {
    if (!do_file(argv[i])) {
      fprintf(stderr, "%s: failed to execute '%s'\n", argv[0], argv[i]);
      goto fail;
    }
  }

  /* enter server or interactive mode. */
#ifdef __VFL_USE_LIBUV
  if (server) {
    if (!comm_server_run(server))
      fprintf(stderr, "%s: shutting down...\n", argv[0]);
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
      if (!do_string(pbuf)) {
        fprintf(stderr, "%s: failed to execute:\n>> %s\n", argv[0], pbuf);
        goto fail;
      }

      /* increment the command counter. */
      if (strcmp(pbuf, ""))
        ncmd++;
    }
  }

  /* indicate successful exit. */
  status = EXIT_SUCCESS;

fail:
  /* free the evaluation string, if allocated. */
  free(evals);

  /* free the client or server, if either is allocated. */
#ifdef __VFL_USE_LIBUV
  comm_client_free(client);
  comm_server_free(server);
  free(hostname);
#endif

  /* return the current status code. */
  return status;
}

