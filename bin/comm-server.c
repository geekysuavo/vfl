
/* include the communication header. */
#include "comm.h"

/* sv_bufalloc(): allocate contents for a read buffer.
 *
 * arguments:
 *  @handle: handle associated with the buffer.
 *  @size: requested size of the allocation.
 *  @buf: pointer to the buffer to manipulate.
 */
static void sv_bufalloc (uv_handle_t *handle, size_t size, uv_buf_t *buf) {
  /* return without allocating if the handle is null. */
  if (!handle)
    return;

  /* allocate exactly the requested size. */
  buf->base = malloc(size);
  buf->len = size;

  /* set the buffer to all zero bytes. */
  memset(buf->base, 0, size);
}

/* sv_onread(): callback function for handling read data from
 * any connected clients.
 *
 * arguments:
 *  @stream: server stream handling the connection.
 *  @nread: number of bytes read, or negative for errors.
 *  @buf: buffer containing the read data from the client.
 */
static void sv_onread (uv_stream_t *stream, ssize_t nread,
                       const uv_buf_t *buf) {
  /* check for read errors and end-of-file. */
  if (nread < 0) {
    /* if the error was not end-of-file related, output a mesage. */
    if (nread != UV_EOF)
      fprintf(stderr, "sv_onread: %s\n", uv_strerror(nread));

    /* in any case, close the stream. */
    uv_close((uv_handle_t*) stream, NULL);
    return;
  }

  /* prepare for handling the read data. */
  comm_server_t *sv = (comm_server_t*) stream->data;
  comm_reply_alloc(&sv->buf, 0, NULL);

  /* call the message callback of the server. */
  int halt = sv->cb(buf->base, &sv->buf);

  /* queue a write of the response created during message handling
   * to the client.
   */
  int ret = uv_write(&sv->rw, stream, &sv->buf, 1, NULL);
  if (ret)
    fprintf(stderr, "uv_write: %s\n", uv_strerror(ret));

  /* free the read data. */
  free(buf->base);

  /* if the message handler signaled that the server should be
   * halted, queue a halt instruction in the main loop.
   */
  if (halt)
    uv_stop(sv->loop);
}

/* sv_connect(): callback function for handling connection requests.
 *
 * arguments:
 *  @server: server stream that received the request.
 *  @status: connection status flag.
 */
static void sv_connect (uv_stream_t *server, int status) {
  /* check for connection receipt errors. */
  if (status < 0) {
    fprintf(stderr, "sv_connect: %s\n", uv_strerror(status));
    return;
  }

  /* prepare for connection acceptance. */
  comm_server_t *sv = (comm_server_t*) server->data;
  uv_tcp_init(sv->loop, &sv->client);
  sv->client.data = (void*) sv;

  /* attempt to accept the client connection request. */
  if (uv_accept(server, (uv_stream_t*) &sv->client) == 0) {
    /* on successful acceptance, queue a read from the client. */
    if (uv_read_start((uv_stream_t*) &sv->client,
                      sv_bufalloc, sv_onread)) return;
  }
  else {
    /* on failed acceptance, close the client handle. */
    uv_close((uv_handle_t*) &sv->client, NULL);
  }
}

/* --- */

/* comm_server_alloc(): allocate a new server structure.
 *
 * arguments:
 *  @hostname: hostname string to bind the server onto.
 *  @cb: response message handler function pointer.
 *
 * returns:
 *  pointer to a newly allocated and initialized server structure.
 */
comm_server_t *comm_server_alloc (const char *hostname,
                                  comm_server_cb cb) {
  /* check the hostname string. */
  if (!hostname)
    return NULL;

  /* allocate a new server structure. */
  comm_server_t *sv = malloc(sizeof(comm_server_t));
  if (!sv)
    return NULL;

  /* prepare for address lookup. */
  sv->loop = uv_default_loop();
  sv->addr.data = (void*) sv;
  sv->cb = cb;

  /* queue an address lookup task. */
  if (uv_getaddrinfo(sv->loop, &sv->addr, NULL,
                     hostname, VFLANG_PORT, NULL)) goto fail;

  /* execute address lookup. */
  if (uv_run(sv->loop, UV_RUN_DEFAULT))
    goto fail;

  /* initialize the server connection information. */
  uv_tcp_init(sv->loop, &sv->conn);
  sv->conn.data = (void*) sv;

  /* bind the server to the first identified address. */
  const struct sockaddr *sa = sv->addr.addrinfo->ai_addr;
  uv_tcp_bind(&sv->conn, sa, 0);

  /* queue a tcp listen task. */
  if (uv_listen((uv_stream_t*) &sv->conn, 128, sv_connect))
    goto fail;

  /* return the new server structure pointer. */
  return sv;

fail:
  /* free the allocated structure and return null. */
  free(sv);
  return NULL;
}

/* comm_server_free(): free an allocated server structure.
 *
 * arguments:
 *  @sv: pointer to the server structure to free.
 */
void comm_server_free (comm_server_t *sv) {
  /* return if the input pointer is null. */
  if (!sv)
    return;

  /* free all allocated server state memory. */
  uv_freeaddrinfo(sv->addr.addrinfo);
  free(sv);
}

/* comm_server_run(): begin accepting clients with a server structure.
 *
 * arguments:
 *  @sv: pointer to the server structure to utilize.
 *
 * returns:
 *  integer indicating success (1) or failure.
 */
int comm_server_run (comm_server_t *sv) {
  /* check the input argument. */
  if (!sv)
    return 0;

  /* execute the server main loop. */
  return (uv_run(sv->loop, UV_RUN_DEFAULT) == 0);
}

