#pragma once
#include "global.h"
#include "mail_t.h"
#include "mt.h"
#include <stddef.h>

#if MAIL_QUEUE_IMPL == 0
#include "queue/moodycamel.hpp"
#elif MAIL_QUEUE_IMPL == 1
#include "queue/locks.h"
#endif

void print_mail (char *prefix, mail_t *mail);
void stringify_mail (mail_t *mail, char *buffer);

typedef struct
{
  postbox_t *boxes;
  size_t b_size;
  size_t *subs[MT_SIZE];
  size_t *subi;
  size_t s_size[MT_SIZE];
} post_office_t;
