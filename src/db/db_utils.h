#pragma once
#include <sqlite3.h>
#include "log/log.h"
#include "sha/sha.h"

typedef enum
{
  DB_EDATA,
  DB_ELOGS,
  DB_EERRORS
} entry_e;

#define SHA256_HASH_LENGTH 64
#define DB_ENTRY_MAX_LEN 600

#if defined(__cplusplus)
extern "C"
{
#endif

  void db_get_time (db_time_t *db_time);
  int db_sql_log (sqlite3_stmt **stmt, log_t *log, db_time_t *db_time, sha_ctx_t *sha_ctx,
                  unsigned char sha_output[SHA256_HASH_LENGTH+1],
                  char entry_buffer[DB_ENTRY_MAX_LEN]);
#if defined(__cplusplus)
}
#endif
