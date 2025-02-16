#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <std/std.h>

static inline float
mean_mov (float mean_prev, size_t n, float x)
{
  /* return (x + n * mean_prev) / (float)(n + 1); */
  /* return mean_prev * (n / (n + 1)) + x / (n + 1); */
  /* return (mean_prev * n + x) / (n + 1); */
  UNUSED (n);
  return mean_prev * 0.7f + x * 0.3f;
  /* (void)n; */
  /* return (x + mean_prev) / 2.0f; */
}

int
entropy (entry_t *cus, float x)
{
  return 0;
}

static size_t time = 0;
int
cusum (cusum_t *cus, float x)
{
  int ret = 0;

  /* printf ("\n\nold mean: %f\n", cus->mean); */
  /* printf ("\n\nprev x: %f\n", cus->x_prev); */
  /* printf ("\n\nx: %f\n", x); */

  /* https://en.wikipedia.org/wiki/Standard_deviation */
  time += 1;
  /* printf ("\n--------------------\nTIME: %ld\n", time); */
  cus->mean = (cus->mean_i++ == 0) ? x : mean_mov (cus->mean, cus->mean_i, x);
  const float a_tmp = cus->a;
  cus->a = cus->a + (x - cus->a) / cus->mean_i;
  cus->q = cus->q + (x - a_tmp) * (x - cus->a);
  cus->est_std_dev
      = (cus->mean_i == 1) ? 0 : sqrtf (cus->q / (cus->mean_i - 1));
  /* printf ("\n\nest_std_dev: %f\n", cus->est_std_dev); */

  /* printf ("\n\nmean: %f\n", cus->mean); */
  const float Sh = fmaxf (0.0f, cus->Sh_prev + (x - cus->mean)
                                    - cus->shift_detect * cus->est_std_dev),
              Sl = fminf (0.0f, cus->Sl_prev + (x - cus->mean)
                                    + cus->shift_detect * cus->est_std_dev);
  /* printf ("\n\nx: %f\n", x); */
  /* printf ("\n\nSh: %f\n", Sh); */
  /* printf ("\n\nSl: %f\n", Sl); */
  const float limit = cus->h * cus->est_std_dev;
  /* printf ("mean: %f\n", cus->mean); */
  /* printf ("limit: %f\n", limit); */
  /* printf ("std: %f\n", cus->est_std_dev); */
  /* printf ("CUSUM: SH: %f\n", Sh); */
  /* printf ("CUSUM: Sl: %f\n", Sl); */
  /* fflush (stdout); */

#define EPS 0.1
  if (Sh - EPS < 0.0 && Sl + EPS > 0.0)
#undef EPS
    ret = 0;
  else if (Sh > limit)
    ret = 1;
  else if (Sl < -limit)
    ret = -1;

  cus->Sh_prev = Sh;
  cus->Sl_prev = Sl;
  cus->x_prev = x;

  return ret;
}
