
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

/* comm_server_cb(): callback function for building server responses.
 *
 * arguments:
 *  @msg: message string receive from the client.
 *  @buf: output buffer to return to the client.
 *
 * returns:
 *  integer indicating whether (1) or not (0) to halt loop execution.
 */
typedef int (*comm_server_cb) (const char *msg, uv_buf_t *buf);

/* comm_reply_t: structure for holding server responses to client
 * messages.
 */
typedef struct {
  /* @status: indication of interpreter success or failure.
   * @str: string output from the interpreter, if any.
   */
  int32_t status;
  char *str;
}
comm_reply_t;

/* comm_client_t: structure for holding communication client information.
 */
typedef struct {
  /* fundamental client objects:
   *  @loop: networking event loop.
   */
  uv_loop_t *loop;

  /* tcp/ip address resolution and connection variables:
   *  @addr: hostname resolution information.
   *  @conn: tcp connection request structure.
   *  @socket: tcp connection handle.
   */
  uv_getaddrinfo_t addr;
  uv_connect_t conn;
  uv_tcp_t socket;

  /* network i/o variables:
   *  @rw: write request structure.
   *  @buf: write buffer data.
   */
  uv_write_t rw;
  uv_buf_t buf;

  /* linkages to the outside world:
   *  @msg: last message string sent from the client.
   *  @ret: return status code received from the server.
   */
  char *msg;
  int ret;
}
comm_client_t;

/* comm_server_t: structure for holding communication server information.
 */
typedef struct {
  /* fundamental server objects:
   *  @loop: networking event loop.
   */
  uv_loop_t *loop;

  /* tcp/ip address resolution and connection variables:
   *  @addr: hostname resolution information.
   *  @conn: tcp connection request structure.
   *  @client: tcp connection handle.
   */
  uv_getaddrinfo_t addr;
  uv_tcp_t conn, client;

  /* network i/o variables:
   *  @rw: write request structure.
   *  @buf: write buffer data.
   */
  uv_write_t rw;
  uv_buf_t buf;

  /* linkages to the outside world:
   *  @cb: callback function for handling server responses.
   */
  comm_server_cb cb;
}
comm_server_t;

/* function declarations, response handling (comm-reply.c): */

int comm_reply_alloc (uv_buf_t *buf, const int status, const char *str);

void comm_reply_free (uv_buf_t *buf);

int comm_reply_set_status (uv_buf_t *buf, const int status);

int comm_reply_append_str (uv_buf_t *buf, const char *str);

comm_reply_t *comm_reply (char *addr);

/* function declarations, client-side (comm-client.c): */

comm_client_t *comm_client_alloc (const char *hostname);

void comm_client_free (comm_client_t *cl);

int comm_client_send (comm_client_t *cl, const char *msg);

int comm_client_send_file (comm_client_t *cl, const char *fname);

/* function declarations, server-side (comm-server.c): */

comm_server_t *comm_server_alloc (const char *hostname,
                                  comm_server_cb cb);

void comm_server_free (comm_server_t *sv);

int comm_server_run (comm_server_t *sv);

#endif /* !__VFLANG_COMM_H__ */

