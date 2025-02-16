#pragma once
#include "mail/mail_t.h"
#include <concurrentqueue/blockingconcurrentqueue.h>
#include <readerwriterqueue/readerwriterqueue.h>

typedef struct
{
  moodycamel::BlockingConcurrentQueue<mail_t> *box;
} postbox_single_t;

typedef struct
{
  moodycamel::BlockingConcurrentQueue<mail_t> *nm;
  moodycamel::BlockingConcurrentQueue<mail_t> *postman;
  int id;
} postbox_t;

#define MAIL_RECEIVE(msgbox, mail) msgbox->nm->wait_dequeue (mail)
#define MAIL_RECEIVE_POSTMAN(msgbox, mail) msgbox.postman->wait_dequeue (mail)

#define MAIL_SEND(msgbox, mail) msgbox->postman->enqueue (mail)
#define MAIL_SEND_SF(msgbox, mail) msgbox.postman->enqueue (mail)
#define MAIL_SEND_NM(msgbox, mail) msgbox.nm->enqueue (*mail)
#define MAIL_SEND_SINGLE(msgbox, mail) msgbox->box->enqueue (mail);

#define POSTBOX_CONCURRENT_INIT(align, size)                                  \
  new (std::align_val_t (align))                                              \
      moodycamel::BlockingConcurrentQueue<mail_t> (size)
