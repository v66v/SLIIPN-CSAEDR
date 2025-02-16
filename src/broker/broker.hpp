#pragma once
#include "can/can.h"
#include "post_office/post_office.hpp"

typedef struct
{
  can_t can;
  postbox_t *msgbox;
} broker_t;

void *broker (void *vargp);
