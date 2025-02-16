#include "ui.hpp"
#include "external/global.h"
#include "global.hpp"
#include "post_office/post_office.hpp"
#include <std/std-dbg.h>
#include <std/std-macro.h>
#include <string.h>
#include <unistd.h>


static mail_t log_mail = { .priority = 1,
                           .type = MT_STORAGE,
                           .data = { .type = DATA_LOG,
                                     .data = { .log = { .type = LOG_UI,
                                                        .data = { .log = "" },
                                                        .db_time = {} } } },
                           .info = "" };

void
add_message (msg_type type, char *msg)
{
  // if the process has catched up with the thread resert the indexes
  if (ext.mem->index == ext.mem->index_e)
    {
      ext.mem->index = 0;
      ext.mem->index_e = 0;
    }

  ext.mem->msgs[ext.mem->index].type = type;
  strcpy (ext.mem->msgs[ext.mem->index].data, msg);

  // if at max capacity overwrite last entry
  if (ext.mem->index < SHM_DEFAULT_SIZE)
    {
      ext.mem->index++;
    }
}

void *
ui (void *vargp)
{
  postbox_t *msgbox = (postbox_t *)vargp;
  mail_t in;
  char mail_buf[MAIL_DATA_SIZE];

  if (shm_init (&ext.shd, sizeof (*ext.mem), IPC_CREAT))
    return NULL;
  memset (ext.shd.ptr, 0, sizeof (*ext.mem)); // just in case

  // short hand, in order to not constantly cast
  ext.mem = (mem_t *)ext.shd.ptr;

  pthread_mutex_init (&ext.mem->mutex, NULL);

  pthread_mutex_lock (&ext.mem->mutex);

  // ext.mem->msgs[0].type = MSG_NOTIF;
  // strcpy (ext.mem->msgs[0].data, "EDR ONLINE");
  // ext.mem->index = 1;

  pthread_mutex_unlock (&ext.mem->mutex);

  post_office_subscribe (msgbox->id, (MT)(MT_UI | MT_ASSIST));
  while (1)
    {
      MAIL_RECEIVE (msgbox, in);

      stringify_mail (&in, mail_buf);

      if (in.data.type == DATA_MSG)
        {
          if (strcmp (in.info, "NOTIF") == 0)
            {
              pthread_mutex_lock (&ext.mem->mutex);
              add_message (MSG_NOTIF, in.data.data.msg);
              pthread_mutex_unlock (&ext.mem->mutex);
            }
          else if (strcmp (in.info, "ERROR") == 0)
            {
              pthread_mutex_lock (&ext.mem->mutex);
              add_message (MSG_ERROR, in.data.data.msg);
              pthread_mutex_unlock (&ext.mem->mutex);
            }
          else if (strcmp (in.info, "ASSIST") == 0)
            {
              pthread_mutex_lock (&ext.mem->mutex);
              add_message (MSG_ASSISTANCE, in.data.data.msg);

              // ignore all previous, out of time messages
              ext.mem->index_assist = 0;
              pthread_mutex_unlock (&ext.mem->mutex);

              {
                struct timespec delta
                    = { .tv_sec = SLEEP_DEFAULT, .tv_nsec = 0 };

                int breakp = 0;
                while (nanosleep (&delta, &delta)
                       && breakp == 0) // sigalarm cancels sleep
                  { // sigalarm awoke the thread, may as well check for an
                    // answer
                    pthread_mutex_lock (&ext.mem->mutex);
                    if (ext.mem->index_assist > 0)
                      breakp = 1;
                    pthread_mutex_unlock (&ext.mem->mutex);
                  }
              }

              pthread_mutex_lock (&ext.mem->mutex);
              if (ext.mem->index_assist > 0)
                {
                  ext.mem->index_assist--;

                  if (strcmp (ext.mem->assist[ext.mem->index_assist].data,
                              "YES")
                      == 0)
                    {
                      in.type = MT_CELLULAR;
                      in.data.type = DATA_INT;
                      in.data.data.i = 1; // 1 => call for and ambulance
                      MAIL_SEND (msgbox, in);

                      strcpy (log_mail.data.data.log.data.log,
                              "CALLING AMBULANCE");
                      MAIL_SEND (msgbox, log_mail);

                      add_message (MSG_NOTIF, log_mail.data.data.log.data.log);
                      // DEBUG_LOG ("CALLING AMBULANCE\n");
                    }
                  else if (strcmp (ext.mem->assist[ext.mem->index_assist].data,
                                   "NO")
                           == 0)
                    {
                      strcpy (log_mail.data.data.log.data.log,
                              "AMBULANCE AVERTED");
                      MAIL_SEND (msgbox, log_mail);

                      add_message (MSG_NOTIF, log_mail.data.data.log.data.log);
                      // DEBUG_LOG ("AMBULANCE AVERTED\n");
                    }
                }
              else
                {
                  in.type = MT_CELLULAR;
                  in.data.type = DATA_INT;
                  in.data.data.i = 2; // 1 => call for an ambulance
                  MAIL_SEND (msgbox, in);

                  strcpy (log_mail.data.data.log.data.log,
                          "CALLING AMBULANCE, NO ANSWER");
                  MAIL_SEND (msgbox, log_mail); // log the event

                  add_message (MSG_ASSISTANCE_REVERT,
                               log_mail.data.data.log.data.log);
                  // DEBUG_LOG ("AMBULANCE NO ANSWER\n");
                }

              pthread_mutex_unlock (&ext.mem->mutex);
            }
        }
    }
  return NULL;
}
