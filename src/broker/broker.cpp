#include "broker.hpp"
#include "data/index.hpp"
#include "hex/hex.h"
#include "sha/hash_vld.h"
#include "threads/thread.hpp"
#include <std/std-dbg.h>
#include <std/std-str.h>
#include <stdlib.h>
#include <string.h>

static mail_t mail
    = { .priority = 1,
        .type = MT_SAFETY,
        .data = { .type = DATA_FLOAT, .data = { .f = 0.0f } },
        .info = "" },
    log_mail
    = { .priority = 1,
        .type = MT_STORAGE,
        .data
        = { .type = DATA_LOG,
            .data = { .log = { .type = LOG_EVENT_ERROR,
                               .data = { .error = { .errc = 0, .errs = "" } },
                               .db_time = {} } } },
        .info = "" };

static char hex[FLOAT_LENGTH + 1] = { 0 };
static can_msg cmsg = { { 0 }, { 0 }, { 0 } };

void
broker_receive (char *msg, int len, void *data)
{
  broker_t *brokerd = (broker_t *)data;
  cmsg = can_rcv_msg (msg, len);
  tolowers (cmsg.can_data);

  strncpy (hex, cmsg.can_data, FLOAT_LENGTH);

  if (validate_hash (cmsg, hex) == 0)
    {
      const int pid = atoi (cmsg.can_id);

      if (pid == 0)
        {
          log_mail.data.data.log.data.error.errc = DB_ERRORC_INVLD_PID;
          snprintf (log_mail.data.data.log.data.error.errs, LOG_DATA_SIZE,
                    "Brocker: Unable to parse pid: %." STR (CAN_ID_LENGTH) "s",
                    cmsg.can_id);
          MAIL_SEND (brokerd->msgbox, log_mail);
          DEBUG_LOG ("Brocker: Unable to parse pid: '%s'\n", cmsg.can_id);
          return;
        }

      *mail.info = pid;
      mail.data.data.f = hex_get_float (hex);
      MAIL_SEND (brokerd->msgbox, mail);
    }
  else
    {
      log_mail.data.data.log.data.error.errc = DB_ERRORC_INVLD_HSH;
      snprintf (log_mail.data.data.log.data.error.errs, LOG_DATA_SIZE,
                "Invalid message hash: '%s'", cmsg.can_data);
      DEBUG_LOG ("Invalid message hash: '%s'\n", cmsg.can_data);
      MAIL_SEND (brokerd->msgbox, log_mail);
    }
}

void *
broker (void *vargp)
{
  broker_t *brokerd = (broker_t *)vargp;

  // register callback and run can receive
  can_run (&brokerd->can, (void *)broker_receive, (void *)brokerd);
  return NULL;
}
