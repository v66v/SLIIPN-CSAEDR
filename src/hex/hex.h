#pragma once
#include "can/can_t.h"

#if defined(__cplusplus)
extern "C"
{
#endif

  extern unsigned char hex_uc[FLOAT_LENGTH / 2 + 1];
  float hex_get_float (char *hex);

#if defined(__cplusplus)
}
#endif
