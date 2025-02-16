#pragma once
#include "log/log.h"
#include <sqlite3.h>

#if defined(__cplusplus)
extern "C"
{
#endif

  typedef struct
  {
    sqlite3 *sql;
    int rc;
    char *err_msg;
    sqlite3_stmt *stmt[3];
  } db_t;

#define DB_INIT_SERVER(db, db_name) sqlite_local_db_init (db, db_name)
#define DB_INIT(db, db_name) sqlite_local_db_init (db, db_name)
  int sqlite_local_db_init (db_t *db, const char *db_name);

#define DB_EXIT(db) sqlite_local_db_exit (db)
  void sqlite_local_db_exit (db_t *db);

#define DB_LOG(db, log) sqlite_local_db_log (db, log)
  int sqlite_local_db_log (db_t *db, log_t *log);

  int sqlite_local_db_exec_query (db_t *db, const char *entry_buffer);

#if defined(__cplusplus)
}
#endif
