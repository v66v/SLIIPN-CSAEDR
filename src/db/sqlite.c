#include "sqlite.h"
#include "global.h"
#include "sqlite_base.h"
#include <std/std-dbg.h>
#include <std/std-err.h>
#include <std/std-macro.h>
#include <string.h>

static char entry_buffer[DB_ENTRY_MAX_LEN];
static unsigned char sha_output[SHA256_HASH_LENGTH + 1];
static sha_ctx_t sha_ctx = { 0 };
static db_time_t db_time;

const char *db_sql_init_query
    = "CREATE TABLE IF NOT EXISTS LOGS(timestamp_sql text, "
      "timestamp text, type int, msg text, hash text);"
      "CREATE TABLE IF NOT EXISTS DATA(timestamp_sql text, "
      "timestamp text, pid int, data_type int, data float, hash text);"
      "CREATE TABLE IF NOT EXISTS ERRORS(timestamp_sql text, "
      "timestamp text, type int, msg text, code int, hash text);";

int
sqlite_local_db_init_tables (db_t *db)
{
  if ((db->rc = sqlite3_exec (db->sql, db_sql_init_query, 0, 0, &db->err_msg))
      != SQLITE_OK)
    {
      ERROR ("DB: initializing tables: %s\n", db->err_msg);
      sqlite3_free (db->err_msg);
      sqlite_local_db_exit (db);
      return 1;
    }
  return 0;
}

int
sqlite_local_db_init_statements (db_t *db)
{
  DEBUG_LOG ("DB: Preparing statements\n");
  const char *log = "INSERT INTO LOGS VALUES(STRFTIME('%Y-%m-%d %H:%M:%f', "
                    "'NOW', 'LOCALTIME'), ?, ?, ?, ?)";

  if (sqlite3_prepare_v2 (db->sql, log, strlen (log), &db->stmt[0], NULL)
      != SQLITE_OK)
    {
      ERROR ("DB: preparing log sql statement\n");
      return 1;
    }

  const char *error
      = "INSERT INTO ERRORS VALUES(STRFTIME('%Y-%m-%d %H:%M:%f', "
        "'NOW', 'LOCALTIME'), ?, ?, ?, ?, ?)";

  if (sqlite3_prepare_v2 (db->sql, error, strlen (error), &db->stmt[1], NULL)
      != SQLITE_OK)
    {
      ERROR ("DB: preparing error sql statement\n");
      return 1;
    }

  const char *data = "INSERT INTO DATA VALUES(STRFTIME('%Y-%m-%d %H:%M:%f', "
                     "'NOW', 'LOCALTIME'), ?, ?, ?, ?, ?)";

  if (sqlite3_prepare_v2 (db->sql, data, strlen (data), &db->stmt[2], NULL)
      != SQLITE_OK)
    {
      ERROR ("DB: preparing data sql statement\n");
      return 1;
    }

  return 0;
}

int
sqlite_local_db_init (db_t *db, const char *db_name)
{
  db->sql = 0;

  if ((db->rc = sqlite3_open (db_name, &db->sql)) != SQLITE_OK)
    {
      ERROR ("DB: database initialization failed (%s): %s\n", db_name,
             sqlite3_errmsg (db->sql));
      sqlite_local_db_exit (db);
      return 1;
    }

  return sqlite_local_db_init_tables (db)
         || sqlite_local_db_init_statements (db);
}

void
sqlite_local_db_exit (db_t *db)
{
  if (sqlite3_get_autocommit (db->sql) == 0) /* check if in a transaction */
    {
      if (sqlite3_exec (db->sql, "END TRANSACTION", 0, 0, &db->err_msg)
          != SQLITE_OK)
        {
          ERROR ("DB: executing end transaction: %s\n",
                 db->err_msg); /* do not fail, we are already exiting */
          sqlite3_free (db->err_msg);
        }
    }

  if (db->sql)
    sqlite3_close (db->sql);
}

int
string_replace_char (char *str, const char *orig, const char *rep)
{
  char *pivot = str;
  size_t n;
  for (n = 0; (pivot = strchr (pivot, (int)*orig)) != NULL; n++)
    *pivot++ = *rep;

  return n;
}

#define LOG_MSG "writting log: '%s'\n"

static size_t transact_n = 0;
static log_t db_log
    = { .type = LOG_DB_ERROR,
        .data = { .error = { .errc = DB_ERRORC_LOG, .errs = "" } } };

int
sqlite_local_db_log (db_t *db, log_t *log)
{
  int ret = 0;

  if (transact_n == 0)
    {
      if ((db->rc
           = sqlite3_exec (db->sql, "BEGIN TRANSACTION", 0, 0, &db->err_msg))
          != SQLITE_OK)
        {
          ERROR ("DB: begining sql transaction: %s\nReason: %s\n",
                 entry_buffer, db->err_msg);
          sqlite3_free (db->err_msg);
        }
    }

  if (db_sql_log (db->stmt, log, &db_time, &sha_ctx, sha_output, entry_buffer))
    {
      string_replace_char (entry_buffer, "\"", "'"); /* sanitize string */

      snprintf (db_log.data.error.errs, LOG_DATA_SIZE, LOG_MSG, entry_buffer);
      sqlite_local_db_log (db, &db_log);

      DEBUG_LOG ("'%s'\n", db_log.data.error.errs);

      ret = 1;
    }

  transact_n++;

  if (transact_n >= TRANSACTION_COUNT)
    {
      transact_n = 0;
      if ((db->rc
           = sqlite3_exec (db->sql, "END TRANSACTION", 0, 0, &db->err_msg))
          != SQLITE_OK)
        {
          ERROR ("DB: ending sql transaction: %s\nReason: %s\n", entry_buffer,
                 db->err_msg);
          sqlite3_free (db->err_msg);
        }
    }

  return ret;
}

int
sqlite_local_db_exec_query (db_t *db, const char *entry_buffer)
{
  if ((db->rc = sqlite3_exec (db->sql, entry_buffer, 0, 0, &db->err_msg))
      != SQLITE_OK)
    {
      ERROR ("DB: executing sql query: '%s': %s\n", entry_buffer, db->err_msg);
      sqlite3_free (db->err_msg);
      return 1;
    }

  return 0;
}
