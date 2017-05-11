
/* include the communication header. */
#include "comm.h"

/* comm_reply_alloc(): allocate a response within a buffer.
 *
 * arguments:
 *  @buf: buffer to allocate the response into.
 *  @status: initial status code to set.
 *  @str: initial string value to set.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int comm_reply_alloc (uv_buf_t *buf, const int status, const char *str) {
  /* check the input buffer. */
  if (!buf)
    return 0;

  /* initialize the buffer contents. */
  buf->base = NULL;
  buf->len = 0;

  /* determine the allocation requirement. */
  const size_t bytes = sizeof(comm_reply_t) + (str ? strlen(str) : 0) + 1;

  /* allocate memory for the response. */
  char *addr = malloc(bytes);
  if (!addr)
    return 0;

  /* initialize the reply structure members. */
  comm_reply_t *reply = comm_reply(addr);
  reply->status = status;

  /* initialize the reply string contents. */
  strcpy(reply->str, "");
  if (str)
    strcat(reply->str, str);

  /* store the new buffer contents. */
  buf->base = addr;
  buf->len = bytes;

  /* return success. */
  return 1;
}

/* comm_reply_free(): free the contents of an allocated reply buffer.
 *
 * arguments:
 *  @buf: buffer to free.
 */
void comm_reply_free (uv_buf_t *buf) {
  /* check the input buffer. */
  if (!buf)
    return;

  /* free the buffer contents. */
  free(buf->base);
  buf->base = NULL;
  buf->len = 0;
}

/* comm_reply_set_status(): set the status of a reply buffer.
 *
 * arguments:
 *  @buf: buffer to store the status into.
 *  @status: new status code to set.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int comm_reply_set_status (uv_buf_t *buf, const int status) {
  /* check the input buffer and its contents. */
  if (!buf || !buf->base)
    return 0;

  /* set the status and return success. */
  comm_reply_t *reply = comm_reply(buf->base);
  reply->status = status;
  return 1;
}

/* comm_reply_append_str(): extend the message string of a reply buffer.
 *
 * arguments:
 *  @buf: buffer to store the status into.
 *  @str: string to append to the reply.
 *
 * returns:
 *  integer indicating success (1) or failure (0).
 */
int comm_reply_append_str (uv_buf_t *buf, const char *str) {
  /* check the input buffer and its contents. */
  if (!buf || !buf->base || !buf->len)
    return 0;

  /* determine the allocation requirement. */
  size_t bytes = buf->len + (str ? strlen(str) : 0);

  /* reallocate memory for the extended response. */
  char *addr = realloc(buf->base, bytes);
  if (!addr)
    return 0;

  /* append the new string to the response message. */
  comm_reply_t *reply = comm_reply(addr);
  strcat(reply->str, str);

  /* store the new buffer contents. */
  buf->base = addr;
  buf->len = bytes;

  /* return success. */
  return 1;
}

/* comm_reply(): cast a memory buffer to a reply.
 *
 * arguments:
 *  @addr: memory location to interpret as a reply.
 *
 * returns:
 *  reply structure pointer.
 */
comm_reply_t *comm_reply (char *addr) {
  /* cast the memory address to a reply structure and update
   * its message string pointer.
   */
  comm_reply_t *reply = (comm_reply_t*) addr;
  reply->str = addr + sizeof(comm_reply_t);

  /* return the reply structure. */
  return reply;
}

