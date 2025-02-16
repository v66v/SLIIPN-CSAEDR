#pragma once
#include "db/db.h"
#include "post_office/post_office.hpp"

typedef struct
{
  db_t *db;
  postbox_t *msgbox;
} logger_t;

void *logger (void *vargp);
