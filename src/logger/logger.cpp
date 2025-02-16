#include "logger.hpp"
#include "db/db_utils.h"
#include "global.hpp"
#include "log/log.h"
#include <signal.h>
#include <std/std-dbg.h>
#include <string.h>

void *
logger (void *vargp)
{
  logger_t *logger = (logger_t *)vargp;
  mail_t in;
  post_office_subscribe (logger->msgbox->id, (MT)(MT_STORAGE));

  while (1)
    {
      MAIL_RECEIVE (logger->msgbox, in);

      if (in.type == MT_STORAGE && in.data.type == DATA_LOG)
        {
          if (in.data.data.log.type == LOG_EVENT_CRASH_MID
              || in.data.data.log.type == LOG_EVENT_CRASH_END)
            {
              db_time_t db_time;
              db_get_time (&db_time);
              snprintf (in.data.data.log.data.log
                            + strlen (in.data.data.log.data.log),
                        LOG_DATA_SIZE, " %s", db_time.timestamp);
            }

          if (DB_LOG (logger->db, &in.data.data.log) != 0)
            {
              DEBUG_ERROR ("writting failed\n");
            }

          if (in.data.data.log.type == LOG_EVENT_CRASH_MID)
            {
              TRANSACTION_COUNT = TRANSACTION_COUNT_AFTER;
              DEBUG_LOG ("TRANSACTION_COUNT after: %d\n", TRANSACTION_COUNT);
            }
          else if (in.data.data.log.type == LOG_EVENT_CRASH_END)
            {
              DEBUG_LOG ("Exiting...\n");
              crashed = 1;

              { // sleep indefinitely, but awake for signals
                sigset_t mask;
                sigemptyset (&mask);
                while (1)
                  sigsuspend (&mask);
              }
            }
        }
      else
        {
          DEBUG_LOG ("mail NOT OK\n");
        }
    }

  return NULL;
}
