#include "hex.h"
#include <std/std-dbg.h>
#include <std/std-macro.h>
#include <stdlib.h>
#include <string.h>

#define FLOAT_STR_LENGTH 11
char float_s[FLOAT_STR_LENGTH] = { 0 };
unsigned int flt[2];

float
hex_get_float (char *hex)
{
  if (sscanf (hex, "%" STR (FLOAT_LENGTH_2) "x%" STR (FLOAT_LENGTH_2) "x", flt,
              flt + 1)
      < 2)
    {
      DEBUG_ERROR ("hex: invalid format: %s\nSetting to zero\n", hex);
      flt[0] = 0;
      flt[1] = 0;
    }

  /* printf ("Hex: %s\n", hex); */

  int LSB = flt[0] & 1;
  unsigned int result = (unsigned int)flt[0] >> 1;
  if (LSB == 1)
    {
      flt[0] = -result;
    }
  else
    {
      flt[0] = result;
    }

  snprintf (float_s, FLOAT_STR_LENGTH - 1, "%d.%04d", flt[0], flt[1]);

  float_s[FLOAT_STR_LENGTH - 1] = '0';
  /* printf ("Received value: %s\n", float_s); */

  return (float)atof (float_s);
}
