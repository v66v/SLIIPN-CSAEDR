#pragma once
#include "log/log.h"
#include <arpa/inet.h>
#include <sqlite3.h>

#if defined(__cplusplus)
extern "C"
{
#endif

  typedef struct
  {
    int sockfd;
    int connfd;
    struct sockaddr_in servaddr, cli;
    char *err_msg;
    sqlite3_stmt *stmt[3];
  } db_t;

#define DB_INIT(db, name)                                                     \
  (void)name;                                                                 \
  sqlite_remote_db_init (db)
  int sqlite_remote_db_init (db_t *db);

#define DB_EXIT(db) sqlite_remote_db_exit (db)
  void sqlite_remote_db_exit (db_t *db);

#define DB_LOG(db, log) sqlite_remote_db_log (db, log)
  int sqlite_remote_db_log (db_t *db, log_t *log);

#if defined(__cplusplus)
}
#endif
