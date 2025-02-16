#include "postman.hpp"
#include "mail/mail.hpp"
#include "post_office/post_office.hpp"
#include <errno.h>
#include <math.h>
#include <std/std-dbg.h>
#include <string.h>
#include <unistd.h>

void *
postman (void *vargp)
{
  post_office_subscribe (0, MT_POSTMAN);
  postman_deliver ((post_office_t *)vargp);
  return NULL;
}

void
postman_send (post_office_t *post_office, int type, mail_t *mail)
{
  for (size_t i = 0; i < post_office->s_size[type]; i++)
    {
      MAIL_SEND_NM (post_office->boxes[post_office->subs[type][i]], mail);
    }
}

#define LOG_SUB_STRING "No process subscribed for mail: "
#define LOG_SUB_STRING_LEN strlen (LOG_SUB_STRING)

void
postman_deliver (post_office_t *post_office)
{
  mail_t mail,
      mail_log
      = { .priority = 1,
          .type = MT_STORAGE,
          .data
          = { .type = DATA_LOG,
              .data
              = { .log = { .type = LOG_ERROR,
                           .data = { .error = { .errc = 1, .errs = "" } },
                           .db_time = {} } } },
          .info = "" },
      mail_notif
      = { .priority = 1,
          .type = MT_ASSIST,
          .data = { .type = DATA_MSG, .data = { .msg = "EDR/DA ONLINE" } },
          .info = "NOTIF" };

  MAIL_SEND ((&post_office->boxes[0]), mail_notif); // send message to self

  int type2;

  while (1)
    {
      MAIL_RECEIVE_POSTMAN (post_office->boxes[0], mail);

      if (mail.type == MT_POSTMAN) // EMERGENCY MESSAGE HANDLE IT
        {
          DEBUG_LOG ("handling EMERGENCY message\n");
          continue;
        }

      // if (mail.type == MT_STORAGE) // EMERGENCY MESSAGE HANDLE IT
      //   {
      //     printf ("POSTMAN\n");
      //   }

      type2 = (int)log2 (mail.type);

      if (mail.type == 0 || type2 >= MT_MAX
          || post_office->s_size[type2] == 0) // MT_MAX: prevent out of bounds
        {
          DEBUG_WARN ("No one subscribed to %s\n", mt_name ((MT)mail.type));

          strncpy (mail_log.data.data.log.data.error.errs, LOG_SUB_STRING,
                   LOG_SUB_STRING_LEN + 1);
          stringify_mail (&mail, mail_log.data.data.log.data.error.errs
                                     + LOG_SUB_STRING_LEN);

          postman_send (post_office, (int)log2 ((int)MT_STORAGE), &mail_log);
          continue;
        }

      postman_send (post_office, type2, &mail);
    }
}
