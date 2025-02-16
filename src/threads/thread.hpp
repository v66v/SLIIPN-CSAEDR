#pragma once
#include "broker/broker.hpp"
#include "mail/mail.hpp"
#include <pthread.h>

void threadsc (void *(*func[]) (void *), pthread_t *tids, size_t N,
               postbox_t *postboxes);
void threadsr (void *(*func[]) (void *), size_t tidi, pthread_t *tids,
               size_t N, postbox_t *postboxes);
void threadsk (pthread_t *tids, size_t N);

void *ui (void *vargp);
void *cellular (void *vargp);
