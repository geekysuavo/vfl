
/* include the communication header. */
#include "comm.h"

/* comm_server_alloc(): allocate a new server structure.
 *
 * arguments:
 *  @hostname: hostname string to bind the server onto.
 *
 * returns:
 *  pointer to a newly allocated and initialized server structure.
 */
comm_server_t *comm_server_alloc (const char *hostname) {
  /* check the hostname string. */
  if (!hostname)
    return NULL;

  /* FIXME */ return NULL;
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

