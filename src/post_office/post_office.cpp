#include "post_office.hpp"
#include "global.hpp"
#include "postman/postman.hpp"
#include <std/std-dbg.h>
#include <std/std-err.h>
#include <std/std-lib.h>
#include <string.h>

// Internal to post_office
// post_office_t post_office = {};
post_office_t post_office = {};
pthread_barrier_t post_office_bar;
pthread_mutex_t subscribe_mutex = PTHREAD_MUTEX_INITIALIZER;

post_office_t *
post_office_get_office ()
{
  return &post_office;
}

int
post_office_init (size_t N, size_t barrier_offset)
{
  N++; // for postman
  post_office.b_size = N * sizeof (postbox_t);

  if (mallocp ((void **)&post_office.boxes, post_office.b_size))
    {
      post_office.b_size = 0;
      return 1;
    }

  post_office.b_size = N;

  if (callocp ((void **)&post_office.subi, post_office.b_size,
               sizeof (size_t)))
    {
      return 1;
    }

  post_office_init_postboxes ();
  post_office_init_subscriptions ();

  pthread_barrier_init (&post_office_bar, NULL,
                        post_office.b_size + barrier_offset);
  return 0;
}

void
post_office_get_boxes (postbox_t **p_office, size_t *size)
{
  *p_office = post_office.boxes + 1;
  *size = post_office.b_size - 1;
}

int
post_office_init_subscriptions ()
{
  if (mallocp ((void **)&post_office.subs[0],
               sizeof (size_t) * MT_SIZE * (post_office.b_size)))
    return 1;

  for (size_t i = 1; i < MT_SIZE; i++)
    {
      post_office.subs[i] = &post_office.subs[0][post_office.b_size * i];
    }
  return 0;
}

void
post_office_subscribe (ssize_t id, MT mt)
{
  if (mt == MT_IGNORE)
    goto wait;

  if (post_office.subi[id] != 0) // process got restarted
    {
      DEBUG_LOG ("id: %d got restarted\n", id);
      return;
    }

  post_office.subi[id] = 1;
  for (int j = 0, i = 1 << j; j < MT_SIZE; j++, i = 1 << j)
    {
      if (mt & i)
        {
          pthread_mutex_lock (&subscribe_mutex);
          // DEBUG_LOG("idx: %d\n",post_office.s_size[j]);
          post_office.subs[j][post_office.s_size[j]] = id; // id
          // post_office.subs[post_office.s_size[j]][j] = 69; //id
          post_office.s_size[j]++;
          DEBUG_LOG ("PO: id %ld subscribed to %s (%d)\n", id, mt_name ((MT)i),
                     i);
          pthread_mutex_unlock (&subscribe_mutex);
        }
    }

wait:
  pthread_barrier_wait (&post_office_bar);
}

void
post_office_init_postboxes ()
{
  // src/process/post_office.cpp:97:31: warning: 'new' of type
  // 'moodycamel::BlockingReaderWriterQueue<mail_t>' with extended alignment 64
  // [-Waligned-new=]
  //  97 |           MAILBOX_INITIAL_SIZE);
  //     |                               ^
  // src/process/post_office.cpp:97:31: note: uses 'void* operator
  // new(std::size_t)', which does not have an alignment parameter
  // src/process/post_office.cpp:97:31: note: use '-faligned-new' to enable

  // https://github.com/cameron314/readerwriterqueue/issues/109
  // Says that it is normal
  // FIX: enabling gnu++17 and adding(std::align_val_t(512))
  const int align = 512; // Maybe needs increasing

  post_office.boxes[0].postman
      = POSTBOX_CONCURRENT_INIT (align, MAILBOX_INITIAL_SIZE);

  post_office.boxes[0].id = 0;

  for (size_t i = 1; i < post_office.b_size; i++)
    {
      post_office.boxes[i].nm
          = POSTBOX_CONCURRENT_INIT (align, MAILBOX_INITIAL_SIZE);
      post_office.boxes[i].postman = post_office.boxes[0].postman;
      post_office.boxes[i].id = i;
    }
}

void
post_office_deinit ()
{
  pthread_barrier_destroy (&post_office_bar);
  free (post_office.boxes);
  free (post_office.subs[0]);
}
