#pragma once
#include "shm/shm.h"
#include <pthread.h>

#define SHM_DEFAULT_SIZE 100
#define SLEEP_DEFAULT 10

typedef enum
{
  MSG_NOTIF,
  MSG_ERROR,
  MSG_ASSISTANCE,
  MSG_ASSISTANCE_REVERT,
} msg_type;

typedef struct
{
  msg_type type;
  char data[200];
} msg_t;

typedef struct
{
  pthread_mutex_t mutex;
  msg_t msgs[SHM_DEFAULT_SIZE];
  msg_t assist[SHM_DEFAULT_SIZE / 100];
  size_t index;
  size_t index_e;
  size_t index_assist;
} mem_t;

typedef struct
{
  shm_t shd;
  mem_t *mem;
} mem_external;
