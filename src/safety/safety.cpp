#include "safety.hpp"
#include "da.hpp"
#include "data/index.hpp"
#include "data/utils.h"
#include "post_office/post_office.hpp"
#include "status.hpp"
#include <std/std-dbg.h>
#include <std/std-err.h>
#include <std/std-macro.h>
#include <string.h>

static safety_status status = {};

void *
safety (void *vargp)
{
#if POSTMAN_OPT == 0
  postbox_t *msgbox = (postbox_t *)vargp;
  postbox_single_t mailbox = { .box = msgbox->postman };
#elif POSTMAN_OPT == 1
  safety_t *safetyd = (safety_t *)vargp;
  postbox_t *msgbox = safetyd->msgbox;
  postbox_single_t mailbox = safetyd->logger;
#endif

  mail_t in;

  if (safety_status_init (&status))
    ERROR ("initializing safety");

  post_office_subscribe (msgbox->id, (MT)(MT_SAFETY));

  while (1)
    {
      MAIL_RECEIVE (msgbox, in);

      if (in.type == MT_SAFETY && in.data.type == DATA_FLOAT)
        {
          safety_status_update (&status, &in);

          safety_da_update (&status, msgbox, &in);

          safety_status_log (&status, &mailbox, &in);
        }
    }
  return NULL;
}
