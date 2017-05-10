
/* ensure once-only inclusion. */
#ifndef __VFLANG_COMM_H__
#define __VFLANG_COMM_H__

/* include c library headers. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* include the uv library header. */
#include <uv.h>

/* VFLANG_PORT: default port number of vflang tcp communications.
 */
#define VFLANG_PORT "4115"

/* comm_client_t: structure for holding communication client information.
 */
typedef struct {
  /* FIXME: document! */
  uv_loop_t *loop;

  uv_getaddrinfo_t addr;
  uv_connect_t conn;
  uv_tcp_t socket;

  uv_write_t rw;
  uv_buf_t buf;

  char *msg;
  int ret;
}
comm_client_t;

/* comm_server_t: structure for holding communication server information.
 */
typedef struct {
  /* FIXME: document! */
  uv_loop_t *loop;

  uv_getaddrinfo_t addr;
  uv_tcp_t conn, client;

  uv_write_t rw;
  uv_buf_t buf;
}
comm_server_t;

/* function declarations, client-side (comm-client.c): */

comm_client_t *comm_client_alloc (const char *hostname);

void comm_client_free (comm_client_t *cl);

int comm_client_send (comm_client_t *cl, const char *msg);

int comm_client_send_file (comm_client_t *cl, const char *fname);

/* function declarations, server-side (comm-server.c): */

comm_server_t *comm_server_alloc (const char *hostname);

void comm_server_free (comm_server_t *sv);

int comm_server_run (comm_server_t *sv);

#endif /* !__VFLANG_COMM_H__ */

