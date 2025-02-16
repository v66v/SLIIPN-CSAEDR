#pragma once
#include <stddef.h>

/* ------------------------------ MAIL ROUTE ------------------------------- */
/* #define POSTMAN_OPT 0 /\* route everything through postman *\/ */
#define POSTMAN_OPT 1 /* route safety messages immediately to logger */

/* ------------------------------ MAIL QUEUE ------------------------------- */
#define MAILBOX_INITIAL_SIZE 40000

#define MAIL_QUEUE_IMPL 0 /* Moodycamel */
// #define MAIL_QUEUE_IMPL 1 /* pthread locks */
#define LOCK_SLEEP_DEFAULT 1000

/* --------------------------- DB IMPLEMENTATION --------------------------- */
#define DB_IMPL 0 /* sqlite file */
/* #define DB_IMPL 1 /\* sqlite server *\/ */

/* ---------------------------- DB TRANSACTION ----------------------------- */
extern size_t TRANSACTION_COUNT;
#define TRANSACTION_COUNT_AFTER (TRANSACTION_COUNT)
