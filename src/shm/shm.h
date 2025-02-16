#pragma once
#include <sys/shm.h>

#if defined(__cplusplus)
extern "C"
{
#endif

  typedef struct
  {
    void *ptr;
    int id;
  } shm_t;

  int shm_init (shm_t *shm, size_t size, int flag);
  int shm_deinit (shm_t *shm);

#if defined(__cplusplus)
}
#endif
