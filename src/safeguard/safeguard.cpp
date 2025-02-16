#include "safeguard.hpp"
#include "alarm/alarm.hpp"
#include "broker/broker.hpp"
#include "global.hpp"
#include "logger/logger.hpp"
#include "post_office/post_office.hpp"
#include "postman/postman.hpp"
#include "safety/safety.hpp"
#include "threads/cellular.hpp"
#include "threads/debug.hpp"
#include "threads/thread.hpp"
#include "threads/ui.hpp"
#include <std/std-dbg.h>
#include <std/std-lib.h>
#include <std/std-macro.h>
#include <std/std.h>

// internal
broker_t brokerd = {};
logger_t loggerd = {};

#if POSTMAN_OPT == 0
#define d_size 3
#define box_offset 2
#define safety_box 0
static void *(*df[d_size]) (void *) = { postman, broker, logger };
static char dn[d_size][FUNC_MAX_LEN] = { "postman", "broker", "logger" };
static void *d[d_size] = { NULL, &brokerd, &loggerd };
static size_t dl[d_size] = { 2, 1, 1 };

static void *(*processes[]) (void *) = { ui, cellular, safety };
static char processesn[][FUNC_MAX_LEN] = { "ui", "cellular", "safety" };
static size_t processesl[] = { 5, 5, 5 };

#elif POSTMAN_OPT == 1

#define d_size 4
#define box_offset 3
#define safety_box 1
safety_t safetyd = {};

static void *(*df[d_size]) (void *) = { postman, broker, logger, safety };
static char dn[d_size][FUNC_MAX_LEN]
    = { "postman", "broker", "logger", "safety" };
static void *d[d_size] = { NULL, &brokerd, &loggerd, &safetyd };
static size_t dl[d_size] = { 2, 1, 1, 2 };

static void *(*processes[]) (void *) = { ui, cellular };
static char processesn[][FUNC_MAX_LEN] = { "ui", "cellular" };
static size_t processesl[] = { 5, 5 };

#endif

void
safeguard_init_threads ()
{
  int num_cores = sysconf (_SC_NPROCESSORS_ONLN);
  int cpu_id = 0;

  cpu_set_t cpuset;
  CPU_ZERO (&cpuset);
  for (size_t i = 0; i < d_size; i++)
    {
      CPU_SET (cpu_id, &cpuset);
      if (sched_setaffinity (0, sizeof (cpuset), &cpuset)
          == -1) // TODO: move to certain CPU
        ERROR ("sched_setaffinity");
      const int next_id = cpu_id + 1;
      cpu_id = next_id >= num_cores ? 0 : next_id;

      pthread_create (&safeguard.tids[i], NULL, safeguard.funcs[i], d[i]);
    }

  threadsc (safeguard.funcs + d_size, safeguard.tids + d_size,
            post_office.b_size - box_offset,
            post_office.boxes + box_offset); // boxes - (postman + logger)
}

int
safeguard_init (char *vcan, char *db_name)
{
  signal (SIGALRM, pthread_sigalrm_handler);

  d[0] = post_office_get_office ();

  if (DB_INIT (&safeguard.db, db_name))
    return 1;
  loggerd.db = &safeguard.db;

  const size_t prcs = sizeof (processes);

  if (mallocp ((void **)&safeguard.funcs, prcs + sizeof (df)))
    return 1;

  if (mallocp ((void **)&safeguard.funcn, sizeof (dn) + sizeof (processesn)))
    return 1;

  safeguard.size = d_size + prcs / sizeof (processes[0]);

  if (callocp ((void **)&safeguard.tickcount, safeguard.size, sizeof (size_t)))
    return 1;

  if (callocp ((void **)&safeguard.keep_alive, safeguard.size,
               sizeof (size_at)))
    return 1;

  if (mallocp ((void **)&safeguard.leniency, safeguard.size * sizeof (size_t)))
    return 1;

  if (mallocp ((void **)&safeguard.tids, safeguard.size * sizeof (size_t)))
    return 1;

  for (size_t i = 0; i < d_size; i++)
    {
      safeguard.funcs[i] = df[i];
      safeguard.leniency[i] = dl[i];
    }

  mempcpy (safeguard.funcs + d_size, processes, prcs);
  mempcpy (safeguard.leniency + d_size, processesl, sizeof (processesl));

  mempcpy (safeguard.funcn, dn, sizeof (dn));
  mempcpy (safeguard.funcn + d_size, processesn, sizeof (processesn));

  // ---------------------------------------------v loggers box
  if (post_office_init (safeguard.size - d_size + 1 + safety_box,
                        1)) // +1 for self
    {
      DEBUG_ERROR ("Post office init failed\n");
      return 1;
    }

  brokerd.can = can_init (vcan, can_receive);
  brokerd.msgbox = &post_office.boxes[0]; // copy

  loggerd.msgbox = &post_office.boxes[1]; // actual

#if POSTMAN_OPT == 1
  safetyd.msgbox = &post_office.boxes[2];
  safetyd.logger.box = loggerd.msgbox->nm;
#endif

  safeguard_init_threads ();

  post_office_subscribe (-1,         // ignored
                         MT_IGNORE); // wait for all the process to subscribe
  return 0;
}

void
safeguard_restart_thread (size_t tid)
{
  if (tid > d_size)
    {
      threadsr (safeguard.funcs + d_size, tid, safeguard.tids + d_size,
                post_office.b_size - 2, post_office.boxes + 2);
      return;
    }

  pthread_cancel (safeguard.tids[tid]);
  pthread_create (&safeguard.tids[tid], NULL, safeguard.funcs[tid], d[tid]);
}

#define RESTART_STRING "Restarting process: "
#define RESTART_STRING_LEN strlen (RESTART_STRING)

static mail_t restart_mail
    = { .priority = 1,
        .type = MT_STORAGE,
        .data
        = { .type = DATA_LOG,
            .data
            = { .log = { .type = LOG_EVENT_ERROR,
                         .data = { .error = { .errc = DB_ERRORC_RESTR_PRCS,
                                              .errs = RESTART_STRING } },
                         .db_time = {} } } },
        .info = "" };

void
safeguard_check_responce ()
{
  int tmp;
  for (size_t k = 0; k < safeguard.size; k++)
    {
      if (safeguard.keep_alive[k][0] != safeguard.keep_alive[k][1])
        {
          tmp = atomic_load (&safeguard.keep_alive[k][1]);
          atomic_store (safeguard.keep_alive[k], tmp);
        }
      else
        {
          DEBUG_WARN ("tid: %ld not responding\n", safeguard.tids[k]);
          safeguard.tickcount[k]++;
          if (safeguard.tickcount[k] > safeguard.leniency[k])
            {
              DEBUG_WARN ("restarting tid: %ld\n", safeguard.tids[k]);
              snprintf (restart_mail.data.data.log.data.error.errs
                            + RESTART_STRING_LEN,
                        MAIL_DATA_SIZE - RESTART_STRING_LEN,
                        "%." STR (FUNC_MAX_LEN) "s", safeguard.funcn[k]);

              MAIL_SEND_SF ((*(post_office_t *)d[0]).boxes[0], restart_mail);

              safeguard_restart_thread (k);
              safeguard.tickcount[k] = 0;
            }
        }
    }
}

void
safeguard_run ()
{
  while (1)
    {
      if (crashed == 1)
        return;

      for (size_t i = 0; i < safeguard.size; i++)
        {
          if (pthread_kill (safeguard.tids[i], 0) != ESRCH)
            pthread_kill (safeguard.tids[i], SIGALRM);
        }

      sleep (keep_alive_period);

      safeguard_check_responce ();
    }
}

void
safeguard_update_keep_alive (size_t tid)
{
  atomic_fetch_add (&safeguard.keep_alive[tid][1], 1);
}

void
safeguard_deinit ()
{
  kill (brokerd.can.pid, SIGKILL);
  threadsk (safeguard.tids, safeguard.size);
  post_office_deinit ();
  DB_EXIT (&safeguard.db);
  shm_deinit (&ext.shd);

  free (safeguard.funcs);
  free (safeguard.tickcount);
  free (safeguard.keep_alive);
  free (safeguard.leniency);
  free (safeguard.tids);
}
