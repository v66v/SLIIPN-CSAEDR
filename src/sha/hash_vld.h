#pragma once
#include "can/can.h"

#if defined(__cplusplus)
extern "C"
{
#endif

typedef struct
{
  char input[CAN_ID_LENGTH + FLOAT_LENGTH];
  char expected[CAN_HASH_LENGTH + 1];
  char actual[CAN_HASH_LENGTH + 2]; // 9
  unsigned int actual_len;
} hash_vld;

extern hash_vld hvld;

int validate_hash (can_msg msg, char *hex);

#if defined(__cplusplus)
}
#endif
