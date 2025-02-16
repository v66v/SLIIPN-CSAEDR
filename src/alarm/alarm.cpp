#include "alarm.hpp"
#include "global.hpp"
#include <std/std.h>
#include <threads.h>

void
pthread_sigalrm_handler (int sig)
{
  UNUSED (sig); /* For now we don't check */
  pthread_t tid = pthread_self ();
  size_t i;

  for (i = 0; safeguard.tids[i] != tid && i < safeguard.size; i++)
    ;

  if (i == safeguard.size) /* Not found */
    return;

  safeguard_update_keep_alive (i); // update atomically
}
