#define _GNU_SOURCE
#include "shm.h"
#include <std/std-dbg.h>
#include <std/std-err.h>
#include <stdlib.h>
#include <unistd.h>

#define SHM_DETERMINISTIC

int
shm_init (shm_t *shm, size_t size, int flag)
{
#ifdef SHM_DETERMINISTIC		/* TODO: add the ability to create multiple shms */
  const char *cwd = "SLIIPN-CS-AEDR-SHARED-MEMORY";
#else
  char *cwd = get_current_dir_name ();
#endif

  shm->id = shmget (ftok (cwd, 'a'), size, 0666 | flag); /* IPC_CREAT */

  if (shm->id < 0)
    {
      ERRNO ("Failure accessing shared memory\n");
      return 1;
    }

  shm->ptr = shmat (shm->id, (void *)0, 0);

#ifndef SHM_DETERMINISTIC
  free (cwd);
#endif

  return 0;
}

int
shm_deinit (shm_t *shm)
{
  shmdt (shm->ptr);
  shm->ptr = NULL;

  if (shmctl (shm->id, IPC_RMID, 0))
    {
      ERRNO ("Failure removing shared memory\n");
      return 1;
    }

  return 0;
}
