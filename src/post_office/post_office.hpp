#pragma once
#include "mail/mail.hpp"
#include <unistd.h>

// extern post_office_t post_office;
post_office_t *post_office_get_office ();
int post_office_init (size_t N, size_t barrier_offset);
void post_office_get_boxes (postbox_t **p_office, size_t *size);
int post_office_init_subscriptions ();
void post_office_subscribe (ssize_t id, MT mt);
void post_office_init_postboxes ();
void post_office_deinit ();
