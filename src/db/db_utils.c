#include "db_utils.h"
#include "db/sqlite.h"
#include <std/std-macro.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define ENTRY_FMT "INSERT INTO %s VALUES"
#define ENTRY_FMT_LEN strlen (ENTRY_FMT)

void
db_get_time (db_time_t *db_time)
{
  gettimeofday (&db_time->tm, NULL);
  db_time->tm_info = localtime (&db_time->tm.tv_sec);
  db_time->timestamp_offset = strftime (db_time->timestamp, TIMESTAMP_LEN,
                                        "%Y-%m-%d %H:%M:%S", db_time->tm_info);
  snprintf (db_time->timestamp + db_time->timestamp_offset, TIMESTAMP_LEN,
            ".%03ld", db_time->tm.tv_usec / 1000);
}

static inline int
db_sql_log_elog (sqlite3_stmt **stmt, log_t *log, db_time_t *db_time,
                 sha_ctx_t *sha_ctx,
                 unsigned char sha_output[SHA256_HASH_LENGTH + 1],
                 char entry_buffer[DB_ENTRY_MAX_LEN])
{
  db_get_time (db_time);
  /* DEBUG_LOG ("LOGS\n"); */
  snprintf (entry_buffer, DB_ENTRY_MAX_LEN, "%s%d%s", db_time->timestamp,
            log->type, log->data.log);
  if (sha_hash (sha_ctx, (const unsigned char *)entry_buffer,
                strlen (entry_buffer), (char *)sha_output, SHA256_HASH_LENGTH)
      == 1)
    return 1;

  if (sqlite3_bind_text (*stmt, 1, db_time->timestamp,
                         strlen (db_time->timestamp), SQLITE_STATIC)
      != SQLITE_OK)
    {
      printf ("ERROR binding sql query stamp \n");
      return 1;
    }

  if (sqlite3_bind_int (*stmt, 2, log->type) != SQLITE_OK)
    {
      printf ("ERROR binding sql query type\n");
      return 1;
    }

  if (sqlite3_bind_text (*stmt, 3, log->data.log, strlen (log->data.log),
                         SQLITE_STATIC)
      != SQLITE_OK)
    {
      printf ("ERROR binding sql query log\n");
      return 1;
    }

  if (sqlite3_bind_text (*stmt, 4, (char *)sha_output, SHA256_HASH_LENGTH,
                         SQLITE_STATIC)
      != SQLITE_OK)
    {
      printf ("ERROR binding sql query sha\n");
      return 1;
    }

  int rc;
  if ((rc = sqlite3_step (*stmt)) != SQLITE_DONE)
    {
      printf ("ERROR stepping sql query \"%s\"\n", sqlite3_errstr (rc));
      return 1;
    }

  if ((rc = sqlite3_reset (*stmt)) != SQLITE_OK)
    {
      printf ("ERROR resetting sql query \"%s\"\n", sqlite3_errstr (rc));
      return 1;
    }

  /* snprintf (entry_buffer, DB_ENTRY_MAX_LEN, */
  /*           ENTRY_FMT */
  /*           "(STRFTIME('%%Y-%%m-%%d %%H:%%M:%%f', 'NOW', 'LOCALTIME'), "
   */
  /*           "\"%s\", %d, \"%s\", \"%." STR (SHA256_HASH_LENGTH + 1) "s\");",
   */
  /*           entry.table, db_time->timestamp, log->type, log->data.log,
   */
  /*           sha_output); */
  return 0;
}

static inline int
db_sql_log_eerror (sqlite3_stmt **stmt, log_t *log, db_time_t *db_time,
                   sha_ctx_t *sha_ctx,
                   unsigned char sha_output[SHA256_HASH_LENGTH + 1],
                   char entry_buffer[DB_ENTRY_MAX_LEN])
{

  db_get_time (db_time);
  /* DEBUG_LOG ("ERRORS\n"); */
  snprintf (entry_buffer, DB_ENTRY_MAX_LEN, "%s%d%s%d", db_time->timestamp,
            log->type, log->data.error.errs, log->data.error.errc);
  if (sha_hash (sha_ctx, (const unsigned char *)entry_buffer,
                strlen (entry_buffer), (char *)sha_output, SHA256_HASH_LENGTH)
      == 1)
    return 1;

  if (sqlite3_bind_text (*(stmt + 1), 1, db_time->timestamp,
                         strlen (db_time->timestamp), SQLITE_STATIC)
      != SQLITE_OK)
    {
      printf ("ERROR binding sql query stamp \n");
      return 1;
    }

  if (sqlite3_bind_int (*(stmt + 1), 2, log->type) != SQLITE_OK)
    {
      printf ("ERROR binding sql query type\n");
      return 1;
    }

  if (sqlite3_bind_text (*(stmt + 1), 3, log->data.error.errs,
                         strlen (log->data.error.errs), SQLITE_STATIC)
      != SQLITE_OK)
    {
      printf ("ERROR binding sql query errs\n");
      return 1;
    }

  if (sqlite3_bind_int (*(stmt + 1), 4, log->data.error.errc) != SQLITE_OK)
    {
      printf ("ERROR binding sql query errc\n");
      return 1;
    }

  if (sqlite3_bind_text (*(stmt + 1), 5, (char *)sha_output,
                         SHA256_HASH_LENGTH, SQLITE_STATIC)
      != SQLITE_OK)
    {
      printf ("ERROR binding sql query sha\n");
      return 1;
    }

  int rc = 0;
  if ((rc = sqlite3_step (*(stmt + 1))) != SQLITE_DONE)
    {
      printf ("ERROR stepping sql query \"%s\"\n", sqlite3_errstr (rc));
      return 1;
    }

  if ((rc = sqlite3_reset (*(stmt + 1))) != SQLITE_OK)
    {
      printf ("ERROR resetting sql query \"%s\"\n", sqlite3_errstr (rc));
      return 1;
    }

  /* snprintf ( */
  /*     entry_buffer, DB_ENTRY_MAX_LEN, */
  /*     ENTRY_FMT */
  /*     "(STRFTIME('%%Y-%%m-%%d %%H:%%M:%%f', 'NOW', 'LOCALTIME'), \"%s\",
   * " */
  /*     "%d, \"%s\", %d, \"%." STR (SHA256_HASH_LENGTH + 1) "s\");", */
  /*     entry.table, db_time->timestamp, log->type, log->data.error.errs,
   */
  /*     log->data.error.errc, (char *)sha_output); */
  return 0;
}

static inline int
db_sql_log_edata (sqlite3_stmt **stmt, log_t *log, sha_ctx_t *sha_ctx,
                  unsigned char sha_output[SHA256_HASH_LENGTH + 1],
                  char entry_buffer[DB_ENTRY_MAX_LEN])
{
  /* DEBUG_LOG ("DATA\n"); */
  snprintf (entry_buffer, DB_ENTRY_MAX_LEN, "%s%d%d%f", log->db_time.timestamp,
            log->data.data.pid, log->data.data.type, log->data.data.data.f);
  if (sha_hash (sha_ctx, (const unsigned char *)entry_buffer,
                strlen (entry_buffer), (char *)sha_output, SHA256_HASH_LENGTH)
      == 1)
    return 1;

  /* snprintf (entry_buffer, DB_ENTRY_MAX_LEN, */
  /*           ENTRY_FMT "(STRFTIME('%%Y-%%m-%%d %%H:%%M:%%f', 'NOW', " */
  /*                     "'LOCALTIME'), \"%s\", %d, %d, " */
  /*                     " %f,\"%." STR (SHA256_HASH_LENGTH + 1) "s\");", */
  /*           entry.table, log->db_time.timestamp, log->data.data.pid, */
  /*           log->data.data.type, log->data.data.data.f, sha_output); */

  if (sqlite3_bind_text (*(stmt + 2), 1, log->db_time.timestamp,
                         strlen (log->db_time.timestamp), SQLITE_STATIC)
      != SQLITE_OK)
    {
      printf ("ERROR binding sql query stamp \n");
      return 1;
    }

  if (sqlite3_bind_int (*(stmt + 2), 2, log->data.data.pid) != SQLITE_OK)
    {
      printf ("ERROR binding sql query pid\n");
      return 1;
    }

  if (sqlite3_bind_int (*(stmt + 2), 3, log->data.data.type) != SQLITE_OK)
    {
      printf ("ERROR binding sql query type\n");
      return 1;
    }

  if (sqlite3_bind_double (*(stmt + 2), 4, (double)log->data.data.data.f)
      != SQLITE_OK)
    {
      printf ("ERROR binding sql query f\n");
      return 1;
    }

  if (sqlite3_bind_text (*(stmt + 2), 5, (char *)sha_output,
                         SHA256_HASH_LENGTH, SQLITE_STATIC)
      != SQLITE_OK)
    {
      printf ("ERROR binding sql query sha\n");
      return 1;
    }

  int rc = 0;
  if ((rc = sqlite3_step (*(stmt + 2))) != SQLITE_DONE)
    {
      printf ("ERROR stepping sql query \"%s\"\n", sqlite3_errstr (rc));
      return 1;
    }

  if ((rc = sqlite3_reset (*(stmt + 2))) != SQLITE_OK)
    {
      printf ("ERROR resetting sql query \"%s\"\n", sqlite3_errstr (rc));
      return 1;
    }

  return 0;
}

int
db_sql_log (sqlite3_stmt **stmt, log_t *log, db_time_t *db_time,
            sha_ctx_t *sha_ctx, unsigned char sha_output[SHA256_HASH_LENGTH + 1],
            char entry_buffer[DB_ENTRY_MAX_LEN])
{
  int ret;
  const entry_e type = log->type < LOG_ERROR
                           ? (log->type == LOG_DATA ? DB_EDATA : DB_ELOGS)
                           : DB_EERRORS;

  switch (type)
    {
    case DB_ELOGS:
      ret = db_sql_log_elog (stmt, log, db_time, sha_ctx, sha_output,
                             entry_buffer);
      break;

    case DB_EERRORS:
      ret = db_sql_log_eerror (stmt, log, db_time, sha_ctx, sha_output,
                               entry_buffer);
      break;

    case DB_EDATA:
      ret = db_sql_log_edata (stmt, log, sha_ctx, sha_output, entry_buffer);
      break;

    default:
      ret = 1;
    }

  return ret;
}

/* if ((db->rc = sqlite3_exec (db->sql, entry_buffer, 0, 0,
 * &db->err_msg)) */
/*     != SQLITE_OK) */
/*   { */
/*     fprintf (stderr, "executing sql query: %s\nReason: %s\n",
 * entry_buffer, */
/*              db->err_msg); */
/*     fflush (stderr); */
/*     sqlite3_free (db->err_msg); */
/*     if (recp == 1) /\* if for a 2nd time the query fails, then the db
 */
/*                       internally has an error. Just fail silently *\/
 */
/*       { */
/*         log_t log */
/*             = { .type = LOG_DB_ERROR, */
/*                 .data = { .error = { .errc = DB_ERRORC_LOG, .errs = ""
 * } }
 * }; */

/*         /\* sanitize string*\/ */
/*         string_replace_char (entry_buffer, "\"", "'"); */

/*         snprintf (log.data.error.errs, LOG_DATA_SIZE, LOG_MSG,
 * entry_buffer); */
/*         printf ("'%s'\n", log.data.error.errs); */
/*         sqlite_local_db_log (db, &log, 0); */
/*       } */
/*     else */
/*       { */
/*         DEBUG_ERROR ("executing sql query: %s\nInternal DB, has
 * failed\n",
 */
/*                      entry_buffer); */
/*       } */

/*     return 1; */
/*   } */
