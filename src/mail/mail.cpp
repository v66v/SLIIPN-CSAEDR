#include "mail.hpp"
#include <std/std-macro.h>
#include <string.h>
#include <stdio.h>

void
print_mail (char *prefix, mail_t *mail)
{
  if (mail->data.type == DATA_MSG)
    {
      printf (
          "%s: priority: %5d, type: %5d, data: %." STR (MAIL_DATA_SIZE) "s\n",
          prefix, mail->priority, mail->type, mail->data.data.msg);
    }
  else if (mail->data.type == DATA_FLOAT)
    {
      printf ("%s: priority: %5d, type: %5d, data: %f\n", prefix,
              mail->priority, mail->type, mail->data.data.f);
    }
  else if (mail->data.type == DATA_INT)
    {
      printf ("%s: priority: %5d, type: %5d, data: %i\n", prefix,
              mail->priority, mail->type, mail->data.data.i);
    }
}

void
stringify_mail (mail_t *mail, char *buffer)
{
  if (mail->data.type == DATA_MSG)
    {
      snprintf (buffer, MAIL_DATA_SIZE,
                "priority: %5d, type: %5d, info: %.20s, data: %." STR (
                    MAIL_DATA_SIZE) "s",
                mail->priority, mail->type, mail->info, mail->data.data.msg);
    }
  else if (mail->data.type == DATA_FLOAT)
    {
      snprintf (buffer, MAIL_DATA_SIZE, "priority: %5d, type: %5d, data: %f",
                mail->priority, mail->type, mail->data.data.f);
    }
  else if (mail->data.type == DATA_INT)
    {
      snprintf (buffer, MAIL_DATA_SIZE, "priority: %5d, type: %5d, data: %i",
                mail->priority, mail->type, mail->data.data.i);
    }
}
