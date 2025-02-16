#pragma once
#include "db/db.h"
#include <pthread.h>
#include <stddef.h>

#ifndef __cplusplus
#include <stdatomic.h>
#else
#include <atomic>
#define _Atomic(X) std::atomic<X>
#endif

#define FUNC_MAX_LEN 20
typedef _Atomic (size_t) size_at[2];
typedef char funcname[FUNC_MAX_LEN];

typedef struct
{
  void *(**funcs) (void *);
  char (*funcn)[FUNC_MAX_LEN];
  pthread_t *tids;
  size_t *tickcount;
  size_t size;
  size_t *leniency;
  size_at *keep_alive;
  db_t db;
} safeguard_t;

extern size_t keep_alive_period;
int safeguard_init (char *vcan, char *db_name);
void safeguard_run ();
void safeguard_update_keep_alive (size_t tid);
void safeguard_deinit ();
