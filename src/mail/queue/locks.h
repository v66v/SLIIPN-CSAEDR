#pragma once
#include "mail/mail_t.h"
#include "global.h"

#include <pthread.h>

typedef struct
{
  mail_t *buffer;
  size_t start;
  size_t end;
  size_t capacity;
  size_t scalar;
  pthread_mutex_t mutex;
} queue_mail_t;

typedef struct
{
  queue_mail_t *box;
} postbox_single_t;

typedef struct
{
  queue_mail_t *nm;
  queue_mail_t *em;
  queue_mail_t *postman;
  int id;
} postbox_t;

#if defined(__cplusplus)
extern "C"
{
#endif

  void queue_mail_t_deinit (queue_mail_t *queue);
  int queue_mail_t_init (queue_mail_t *queue, size_t capacity, size_t scalar);
  void queue_mail_t_reset (queue_mail_t *queue);

#define MAIL_RECEIVE(msgbox, mail)                                            \
  {                                                                           \
    struct timespec delta = { .tv_sec = 0, .tv_nsec =  LOCK_SLEEP_DEFAULT };   \
    while (queue_mail_t_pop (msgbox->nm, &mail) == 1)                         \
      while (nanosleep (&delta, &delta))                                      \
        ;                                                                     \
  }

#define MAIL_RECEIVE_POSTMAN(msgbox, mail)                                    \
  {                                                                           \
    struct timespec delta = { .tv_sec = 0, .tv_nsec =  LOCK_SLEEP_DEFAULT };   \
    while (queue_mail_t_pop (msgbox.postman, &mail) == 1)                     \
      while (nanosleep (&delta, &delta))                                      \
        ;                                                                     \
  }

  int queue_mail_t_pop (queue_mail_t *queue, mail_t *item);

#define MAIL_SEND(msgbox, mail) queue_mail_t_add (msgbox->postman, &mail);
#define MAIL_SEND_SINGLE(msgbox, mail) queue_mail_t_add (msgbox->box, &mail);

#define MAIL_SEND_SF(msgbox, mail) queue_mail_t_add (msgbox.postman, &mail)
#define MAIL_SEND_NM(msgbox, mail) queue_mail_t_add (msgbox.nm, mail)

  int queue_mail_t_add (queue_mail_t *queue, mail_t *item);

  void POSTBOX_CONCURRENT_DEINIT (queue_mail_t *mail_queue);

  queue_mail_t *POSTBOX_CONCURRENT_INIT (int align, size_t size);

#if defined(__cplusplus)
}
#endif
