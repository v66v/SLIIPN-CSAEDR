#pragma once
#include "can/can.h"

#if defined(__cplusplus)
extern "C"
{
#endif

  void thread_can_receive (char *msg, int len);

#if defined(__cplusplus)
}
#endif
