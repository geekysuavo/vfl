
/* include the communication header. */
#include "comm.h"

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

  /* FIXME */ return NULL;
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

  /* FIXME */ return 0;
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

  /* FIXME */ return 0;
}

