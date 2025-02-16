#pragma once
#include "log/log.h"

typedef union
{
  char msg[MAIL_DATA_SIZE];
  float f;
  int i;
  log_t log;
} data_u;

typedef struct
{
  data_e type;
  data_u data;
} data_t;

typedef struct
{
  int priority;
  int type;
  data_t data;
  char info[20];
} mail_t;
