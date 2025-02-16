#include "safeguard/safeguard.hpp"
#include <std/std.h>
#include <unistd.h>

int
main (int argc, char *argv[])
{
  if (argc < 3)
    {
      printf ("Usage: main \"vcan_name\" \"db_name\"\n");
      return 1;
    }

  if (safeguard_init (argv[1], argv[2]))
    return 1;

  safeguard_run ();

  safeguard_deinit ();
  return 0;
}
