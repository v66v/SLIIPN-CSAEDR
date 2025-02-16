#include "cbuf_mail.h"
#include <std/std-lib.h>

int
cbuf_mail_init (cbuf_mail_t *cbuf, size_t size)
{
  if (mallocp (&cbuf->buf, size * sizeof (*cbuf->buf)))
    return 1;

  cbuf->size = size;
  return 0;
}

inline void
cbuf_mail_deinit (cbuf_mail_t *cbuf)
{
  freep (&cbuf->buf);
}

inline void
cbuf_mail_reset (cbuf_mail_t *cbuf)
{
  memset (cbuf->buf, 0, cbuf->size * sizeof (*cbuf->buf));
}

void
cbuf_mail_write (cbuf_mail_t *cbuf, mail_t *mail)
{
  memcpy (cbuf->buf + cbuf->index, mail, sizeof (*cbuf->buf));

  if ((size_t)++cbuf->index >= cbuf->size)
    cbuf->index = 0;
}

inline void
cbuf_mail_move_index (cbuf_mail_t *cbuf, size_t n)
{
  const int res = n % cbuf->size;
  cbuf->index -= res;

  if (cbuf->index < 0)
    cbuf->index = cbuf->size - cbuf->index;
}

inline void
cbuf_mail_read_ahead (cbuf_mail_t *cbuf, mail_t *mail)
{
  if ((size_t)++cbuf->index == cbuf->size)
    cbuf->index = 0;

  memcpy (mail, cbuf->buf + cbuf->index, sizeof (*cbuf->buf));
}

inline void
cbuf_mail_read (cbuf_mail_t *cbuf, mail_t *mail)
{
  if (--cbuf->index < 0)
    cbuf->index = cbuf->size - 1;

  memcpy (mail, cbuf->buf + cbuf->index, sizeof (*cbuf->buf));
}
