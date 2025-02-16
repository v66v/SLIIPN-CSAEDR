#pragma once
#include "mail/mail.hpp"
#include <pthread.h>

void *postman (void *vargp);
void postman_deliver (post_office_t *postboxes);
