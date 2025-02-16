#pragma once
#include "post_office/post_office.hpp"

typedef struct
{
  postbox_t *msgbox;
  postbox_single_t logger;
} safety_t;

void *safety (void *vargp);
