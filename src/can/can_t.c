#include "can_t.h"
#include <std/std-macro.h>

char *CAN_CMD[]
    = { (char *)"candump", (char *)"canplayer",
        (char *)"cansend %s %%." STR (CAN_PAYLOAD_LENGTH) "s" };
