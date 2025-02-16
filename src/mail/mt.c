#include "mt.h"
#include <assert.h>

const char *
mt_name (MT mt)
{
  switch (mt)
    {
      MT_ENUM (MT_ENUM_STRING)
    default:
      assert (!"Unknown mt");
    }
}
