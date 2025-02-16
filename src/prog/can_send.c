#include "threads.h"

int
main ()
{
  can_t can1 = can_init ((char *)"vcan0", can_send);
  can_frame frame = { .can_id = "005", .can_data = "3000000003c322b7" };
  can_run (&can1, &frame, NULL);
}
