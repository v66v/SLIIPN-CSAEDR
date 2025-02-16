#include "thread.hpp"
#include "global.hpp"
#include "post_office/post_office.hpp"
#include <iostream>
#include <signal.h>
#include <std/std-dbg.h>
#include <std/std-err.h>
#include <std/std-macro.h>
#include <std/std.h>
#include <stdio.h>
#include <unistd.h>

void
threadsc (void *(*func[]) (void *), pthread_t *tids, size_t N,
          postbox_t *postboxes)
{
  int num_cores = sysconf (_SC_NPROCESSORS_ONLN);
  int cpu_id = 0;

  cpu_set_t cpuset;
  CPU_ZERO (&cpuset);
  for (size_t i = 0; i < N; i++)
    {
      CPU_SET (cpu_id, &cpuset);
      if (sched_setaffinity (0, sizeof (cpuset), &cpuset)
          == -1) // TODO: move to certain CPU
        ERROR ("sched_setaffinity");
      const int next_id = cpu_id + 1;
      cpu_id = next_id >= num_cores ? 0 : next_id;
      pthread_create (&tids[i], NULL, func[i], &postboxes[i]);
    }
}

void
threadsr (void *(*func[]) (void *), size_t tid, pthread_t *tids, size_t N,
          postbox_t *postboxes)
{
  UNUSED (N);
  pthread_cancel (tids[tid]);
  pthread_create (&tids[tid], NULL, func[tid], &postboxes[tid - 1]);
}

void
threadsk (pthread_t *tids, size_t N)
{
  for (size_t i = 0; i < N; i++)
    pthread_cancel (tids[i]);
  // pthread_kill (tids[i], SIGKILL);
}
