#include "threads.h"
#include "sha/hash_vld.h"
#include "hex/hex.h"
#include <std/std-str.h>
#include <std/std.h>
#include <string.h>

void
thread_can_receive (char *msg, int len)
{
  can_msg cmsg = can_rcv_msg (msg, len);
  tolowers (cmsg.can_data);

  // get hex
  char hex[FLOAT_LENGTH + 1] = { 0 };
  strncpy (hex, cmsg.can_data, FLOAT_LENGTH);

  float f = hex_get_float (hex);
  int valid = validate_hash (cmsg, hex);
  UNUSED(f);
  UNUSED(valid);
}
