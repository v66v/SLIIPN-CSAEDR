#include "post_office/post_office.hpp"
#include <std/std-dbg.h>
#include <stdlib.h>
#include <unistd.h>

void *
debug (void *vargp)
{
  unsigned int seed = time (NULL) ^ getpid () ^ pthread_self ();

  postbox_t *msgbox = (postbox_t *)vargp;
  mail_t mail_notif
      = { .priority = 1,
          .type = MT_ASSIST,
          .data
          = { .type = DATA_MSG, .data = { .msg = "System Notification" } },
          .info = "NOTIF" },
      mail_error
      = { .priority = 1,
          .type = MT_ASSIST,
          .data = { .type = DATA_MSG, .data = { .msg = "System failure" } },
          .info = "ERROR" },
      mail_assist
      = { .priority = 1,
          .type = MT_ASSIST,
          .data = { .type = DATA_MSG, .data = { .msg = "Medical attention" } },
          .info = "ASSIST" };

  int rand_num;

  post_office_subscribe (msgbox->id, (MT)(MT_IGNORE));

  while (1)
    {
      rand_num = rand_r (&seed) % 3;

      if (rand_num == 0)
        {
          MAIL_SEND (msgbox, mail_notif);
        }
      else if (rand_num == 1)
        {
          MAIL_SEND (msgbox, mail_error);
        }
      else if (rand_num == 2)
        {
          MAIL_SEND (msgbox, mail_assist);
        }

      {
        struct timespec delta = { .tv_sec = 3, .tv_nsec = 0 };
        while (nanosleep (&delta, &delta)) // sigalarm cancels sleep
          ;
      }
    }
}
