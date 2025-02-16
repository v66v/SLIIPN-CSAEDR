#include "locks.h"

#include <std/std-dbg.h>
#include <std/std-lib.h>
#include <std/std.h>

/* TODO: update libstd and remove */
#if defined(__cplusplus)
extern "C"
{
#endif

  int pthread_mutex_initp (pthread_mutex_t *mutex,
                           const pthread_mutexattr_t *attr);
  int pthread_mutex_destroyp (pthread_mutex_t *mutex);

#if defined(__cplusplus)
}
#endif

void
queue_mail_t_deinit (queue_mail_t *queue)
{
  freep (&queue->buffer);

  queue->capacity = 0;
  queue->scalar = 0;
}

int
queue_mail_t_init (queue_mail_t *queue, size_t capacity, size_t scalar)
{

  queue->start = 0;
  queue->end = 0;

  if (mallocp (&queue->buffer, capacity * sizeof (*queue->buffer)))
    {
      queue->buffer = NULL;
      return 1;
    }

  if (pthread_mutex_initp (&queue->mutex, NULL))
    {
      queue_mail_t_deinit (queue);
      return 1;
    }

  queue->capacity = capacity;
  queue->scalar = scalar;

  return 0;
}

void
queue_mail_t_reset (queue_mail_t *queue)
{
  pthread_mutex_lock (&queue->mutex);
  queue->start = 0;
  queue->end = 0;
  pthread_mutex_unlock (&queue->mutex);
}

int
queue_mail_t_pop (queue_mail_t *queue, mail_t *item)
{
  pthread_mutex_lock (&queue->mutex);
  if (queue->start >= queue->end)
    {
      pthread_mutex_unlock (&queue->mutex);
      return 1;
    }

  memcpy (item, queue->buffer + queue->start, sizeof (*item));
  queue->start++;

  if (queue->start == queue->end)
    {
      queue->start = 0;
      queue->end = 0;
    }

  pthread_mutex_unlock (&queue->mutex);
  return 0;
}

int
queue_mail_t_add (queue_mail_t *queue, mail_t *item)
{
  pthread_mutex_lock (&queue->mutex);
  if (queue->end >= queue->capacity) /* capacity reached */
    {
      DEBUG_ERROR ("Capacity reached\n");
      if (queue->start == 0) /* nothing has been popped */
        {
          if (!queue->capacity || !queue->scalar)
            return 1;

          if (reallocp (&queue->buffer, queue->capacity * queue->scalar))
            {
              pthread_mutex_unlock (&queue->mutex);
              return 1;
            }
          queue->capacity *= queue->scalar;
        }
      else
        {
          DEBUG_ERROR ("Mem move\n");
          memmove (queue->buffer, queue->buffer + queue->start,
                   sizeof (*queue->buffer) * (queue->end - queue->start));
          queue->end -= queue->start;
          queue->start = 0;
        }
    }

  memcpy (queue->buffer + queue->end, item, sizeof (*item));
  queue->end++;

  pthread_mutex_unlock (&queue->mutex);
  return 0;
}

void
POSTBOX_CONCURRENT_DEINIT (queue_mail_t *mail_queue)
{
  queue_mail_t_deinit (mail_queue);
  freep (&mail_queue);
}

queue_mail_t *
POSTBOX_CONCURRENT_INIT (int align, size_t size)
{
  UNUSED (align);
  queue_mail_t *mail_queue;
  if (mallocp (&mail_queue, sizeof (*mail_queue)))
    return NULL;

  if (queue_mail_t_init (mail_queue, size, 2))
    return NULL;

  return mail_queue;
}
