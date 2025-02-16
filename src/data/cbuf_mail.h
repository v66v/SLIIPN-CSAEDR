#pragma once
#include "mail/mail_t.h"
#include <stddef.h>

typedef struct
{
  mail_t *buf;
  size_t size;
  int index;
} cbuf_mail_t;

#if defined(__cplusplus)
extern "C"
{
#endif

  int cbuf_mail_init (cbuf_mail_t *cbuf, size_t size);
  void cbuf_mail_deinit (cbuf_mail_t *cbuf);
  void cbuf_mail_reset (cbuf_mail_t *cbuf);
  void cbuf_mail_write (cbuf_mail_t *cbuf, mail_t *mail);
  void cbuf_mail_read (cbuf_mail_t *cbuf, mail_t *mail);

  void cbuf_mail_move_index (cbuf_mail_t *cbuf, size_t n);
  void cbuf_mail_read_ahead (cbuf_mail_t *cbuf, mail_t *mail);
#if defined(__cplusplus)
}
#endif
