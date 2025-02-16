#pragma once
#include "can_t.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#define CAN_CHECK_STREAM(can)                                                 \
  {                                                                           \
    if (can.stream == NULL)                                                   \
      {                                                                       \
        DEBUG_ERROR ("Failed to run cmd: '%s' for %s\n", can.cmd, can.name);  \
        return 1;                                                             \
      }                                                                       \
  }

  can_t can_init (char *name, can_type type);

  can_msg can_rcv_msg (char *msg, int len);
  void can_send_msg (can_t can, char *tmp, char *payload);

  int can_run_receive (can_t *can, void (*func) (char *, int, void *),
                       void *data);
  int can_run_send (can_t can, can_frame *frame);
  int can_run_play (can_t can, char *filepath);
  int can_run (can_t *can, void *var, void *data);

#if defined(__cplusplus)
}
#endif
