#include "post_office/post_office.hpp"
#include <std/std-dbg.h>
#include <std/std-macro.h>

static mail_t log_mail = { .priority = 1,
                           .type = MT_STORAGE,
                           .data = { .type = DATA_LOG,
                                     .data = { .log = { .type = LOG_UI,
                                                        .data = { .log = "" },
                                                        .db_time = {} } } },
                           .info = "" };

void
get_string (int type, char *buffer)
{
  switch (type)
    {
    case 1:
      snprintf (buffer, MAIL_DATA_SIZE, "patient answered");
      break;
    case 2:
      snprintf (buffer, MAIL_DATA_SIZE, "patient did not respond");
      break;
    default:
      snprintf (buffer, MAIL_DATA_SIZE,
                "Error no such ambulance message type %d\n", type);
      DEBUG_LOG ("Error no such ambulance message type %d\n", type);
    }
}

void *
cellular (void *vargp)
{
  postbox_t *msgbox = (postbox_t *)vargp;
  mail_t in;
  char mail_buf[MAIL_DATA_SIZE];

  post_office_subscribe (msgbox->id, (MT)(MT_CELLULAR));

  while (1)
    {
      MAIL_RECEIVE (msgbox, in);
      stringify_mail (&in, mail_buf);
      // DEBUG_LOG ("recieved mail: %." STR (MAIL_DATA_SIZE) "s\n", mail_buf);

      if (in.type == MT_CELLULAR)
        {
          if (in.data.type == DATA_INT)
            {
              get_string (in.data.data.i, in.data.data.msg);
              DEBUG_LOG ("Calling ambulance with info: %s\n",
                         in.data.data.msg);
              sprintf (log_mail.data.data.log.data.log,
                       "Cellular: CALLING AMBULANCE with info: %s",
                       in.data.data.msg);
              MAIL_SEND (msgbox, log_mail);
            }
          else
            {
              sprintf (log_mail.data.data.log.data.log,
                       "Cellular: incorrect data: %.100s ", in.data.data.msg);
              MAIL_SEND (msgbox, log_mail);
            }
        }
    }
  return NULL;
}
