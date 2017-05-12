
/* include the communication header. */
#include "comm.h"

/* cl_bufalloc(): allocate contents for a read buffer.
 *
 * arguments:
 *  @handle: handle associated with the buffer.
 *  @size: requested size of the allocation.
 *  @buf: pointer to the buffer to manipulate.
 */
static void cl_bufalloc (uv_handle_t *handle, size_t size, uv_buf_t *buf) {
  /* return without allocating if the handle is null. */
  if (!handle)
    return;

  /* allocate exactly the requested size. */
  buf->base = malloc(size);
  buf->len = size;
}

/* cl_onread(): callback function for handling read data from
 * the server, that was sent in response to our message.
 *
 * arguments:
 *  @stream: client stream handling the connection.
 *  @nread: number of bytes read, or negative for errors.
 *  @buf: buffer containing the read data from the server.
 */
static void cl_onread (uv_stream_t *stream, ssize_t nread,
                       const uv_buf_t *buf) {
  /* check for read errors and end-of-file. */
  if (nread < 0) {
    /* if the error was not end-of-file related, output a message. */
    if (nread != UV_EOF)
      fprintf(stderr, "cl_onread: %s\n", uv_strerror(nread));

    /* in any case, close the stream. */
    uv_close((uv_handle_t*) stream, NULL);
    return;
  }

  /* get the client and response structures. */
  comm_client_t *cl = (comm_client_t*) stream->data;
  comm_reply_t *reply = comm_reply(buf->base);

  /* store the status code from the server. */
  cl->ret = reply->status;

  /* strip newlines from the end of the message string. */
  unsigned int n = strlen(reply->str);
  while (n && reply->str[n - 1] == '\n')
    reply->str[(n--) - 1] = '\0';

  /* print the message string from the server. */
  if (n) {
    fprintf(stderr, "%s\n", reply->str);
    fflush(stderr);
  }

  /* close the stream and free the buffer contents. */
  uv_close((uv_handle_t*) stream, NULL);
  comm_reply_free((uv_buf_t*) buf);
}

/* cl_onwrite(): callback function for handling writes to the server.
 *
 * arguments:
 *  @req: write request handle.
 *  @status: write status flag.
 */
static void cl_onwrite (uv_write_t *req, int status) {
  /* check for write errors. */
  if (status < 0) {
    fprintf(stderr, "cl_write: %s\n", uv_strerror(status));
    return;
  }

  /* queue a read to get the server response. */
  req->handle->data = (void*) req->data;
  if (uv_read_start(req->handle, cl_bufalloc, cl_onread))
    return;
}

/* cl_connect(): callback function for handling connection
 * initiations.
 *
 * arguments:
 *  @conn: connection handle.
 *  @status: connection status flag.
 */
static void cl_connect (uv_connect_t *conn, int status) {
  /* check for connection handshaking errors. */
  if (status < 0) {
    fprintf(stderr, "cl_connect: %s\n", uv_strerror(status));
    return;
  }

  /* get a reference to the client structure. */
  comm_client_t *cl = (comm_client_t*) conn->data;

  /* prepare the client for message transmission. */
  cl->buf.base = cl->msg;
  cl->buf.len = strlen(cl->msg);
  cl->rw.data = (void*) cl;

  /* queue a write to the server. */
  if (uv_write(&cl->rw, conn->handle, &cl->buf, 1, cl_onwrite))
    return;
}

/* --- */

/* comm_client_alloc(): allocate a new client structure.
 *
 * arguments:
 *  @hostname: hostname string of the server to connect to.
 *
 * returns:
 *  pointer to a newly allocated and initialized client structure.
 */
comm_client_t *comm_client_alloc (const char *hostname) {
  /* check the hostname string. */
  if (!hostname)
    return NULL;

  /* allocate a new client structure. */
  comm_client_t *cl = malloc(sizeof(comm_client_t));
  if (!cl)
    return NULL;

  /* prepare for address lookup. */
  cl->loop = uv_default_loop();
  cl->addr.data = (void*) cl;

  /* queue an address lookup task. */
  if (uv_getaddrinfo(cl->loop, &cl->addr, NULL,
                     hostname, VFLANG_PORT, NULL)) goto fail;

  /* execute address lookup. */
  if (uv_run(cl->loop, UV_RUN_DEFAULT))
    goto fail;

  /* return the new client structure pointer. */
  return cl;

fail:
  /* free the allocated structure and return null. */
  free(cl);
  return NULL;
}

/* comm_client_free(): free an allocated client structure.
 *
 * arguments:
 *  @cl: pointer to the client structure to free.
 */
void comm_client_free (comm_client_t *cl) {
  /* return if the input pointer is null. */
  if (!cl)
    return;

  /* free all allocated client state memory. */
  uv_freeaddrinfo(cl->addr.addrinfo);
  free(cl);
}

/* comm_client_send(): send a string message to the server that a client
 * was paired with at allocation.
 *
 * arguments:
 *  @cl: pointer to the client structure to access.
 *  @msg: message string to send.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int comm_client_send (comm_client_t *cl, const char *msg) {
  /* check the input arguments. */
  if (!cl || !msg)
    return 0;

  /* return without action for empty message strings. */
  if (strlen(msg) == 0)
    return 1;

  /* initialize the client connection information. */
  uv_tcp_init(cl->loop, &cl->socket);
  cl->conn.data = (void*) cl;

  /* get the information returned by address lookup. */
  const struct sockaddr *sa = cl->addr.addrinfo->ai_addr;

  /* queue a tcp connection task. */
  if (uv_tcp_connect(&cl->conn, &cl->socket, sa, cl_connect))
    return 0;

  /* set the output message and prepare for execution. */
  cl->msg = (char*) msg;
  cl->ret = 0;

  /* execute the connection. */
  if (uv_run(cl->loop, UV_RUN_DEFAULT))
    return 0;

  /* return the final exit status. */
  return cl->ret;
}

/* comm_client_send_file(): send the contents of a file as a message
 * to the server that a client was paired with at allocation.
 *
 * arguments:
 *  @cl: pointer to the client structure to access.
 *  @fname: filename of the message string to send.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int comm_client_send_file (comm_client_t *cl, const char *fname) {
  /* check the input arguments. */
  if (!cl || !fname)
    return 0;

  /* open the input file. */
  FILE *fh = fopen(fname, "r");
  if (!fh)
    return 0;

  /* determine the input file size. */
  fseek(fh, 0, SEEK_END);
  const long sz = ftell(fh);
  fseek(fh, 0, SEEK_SET);

  /* allocate a buffer to store the file contents. */
  char *buf = malloc(sz);
  if (!buf) {
    fclose(fh);
    return 0;
  }

  /* read the file contents into the buffer. */
  fread(buf, 1, sz, fh);

  /* send the file contents to the server. */
  const int ret = comm_client_send(cl, buf);

  /* close the input file, free the buffer, and return. */
  fclose(fh);
  free(buf);
  return ret;
}

