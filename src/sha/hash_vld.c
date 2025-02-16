#include "hash_vld.h"
#include "sha.h"
#include "string.h"
#include <std/std-err.h>

hash_vld hvld = { 0 };
sha_ctx_t sha_ctx = { 0 };

int
validate_hash (can_msg msg, char *hex)
{
  // get hash
  strncpy (hvld.expected, msg.can_data + FLOAT_LENGTH, CAN_HASH_LENGTH);

  // get hashed value
  snprintf (hvld.input, CAN_ID_LENGTH + FLOAT_LENGTH, "%s%s", msg.can_id, hex);

  // hash the value
  sha_hash (&sha_ctx, (const unsigned char *)hvld.input,
            CAN_ID_LENGTH + FLOAT_LENGTH - 1, (char *)hvld.actual,
            CAN_HASH_LENGTH);

  // check if it is valid
  if (strncmp (hvld.expected, hvld.actual, CAN_HASH_LENGTH + 1))
    {
      ERROR ("MSG %s: %s {rec: %s, act: %s} IS INVALID\n", msg.can_id,
             msg.can_data, hvld.expected, hvld.actual);
      return 1;
    }

  /* DEBUG_LOG ("MSG %s: %s IS VALID\n", msg.can_id, msg.can_data); */
  return 0;
}
