#include "can/can.h"
#include <std/std.h>
#include <stdio.h>

int
main (int argc, char *argv[])
{
  UNUSED (argc);
  UNUSED (argv);
  can_t can2 = can_init (argv[2], can_play);
  can_run (&can2, (void *)argv[1], NULL);
  return 22;
}
