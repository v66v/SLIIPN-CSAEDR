#pragma once
#include <stddef.h>

typedef struct
{
  float sum;
} entry_t;

typedef struct
{
  float Sh_prev;
  float Sl_prev;
  float mean;
  size_t mean_i;
  float x_prev;
  float shift_detect;
  float est_std_dev;
  float h;
  float a;
  float q;
} cusum_t;

#if defined(__cplusplus)
extern "C"
{
#endif

  int cusum (cusum_t *cus, float x);

#if defined(__cplusplus)
}
#endif
