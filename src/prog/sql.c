#include <sqlite3.h>
#include <std/std-dbg.h>
#include <stdio.h>

int
main (void)
{

  sqlite3 *db;
  char *err_msg = 0;

  int rc = sqlite3_open ("edr.db", &db);

  if (rc != SQLITE_OK)
    {
      DEBUG_ERROR ("Cannot open database: %s\n", sqlite3_errmsg (db));
      sqlite3_close (db);
      return 1;
    }

  const char *sql = "CREATE TABLE IF NOT EXISTS People(name text, hash text);"
                    "INSERT INTO People VALUES('John Smith', "
                    "'6f8db599de986fab7a21625b7916589c');";

  rc = sqlite3_exec (db, sql, 0, 0, &err_msg);

  if (rc != SQLITE_OK)
    {

      DEBUG_ERROR ("SQL error: %s\n", err_msg);

      sqlite3_free (err_msg);
      sqlite3_close (db);

      return 1;
    }

  sqlite3_close (db);

  return 0;
}
