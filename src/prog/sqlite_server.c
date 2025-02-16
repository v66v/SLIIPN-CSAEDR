#include "db/db_utils.h"
#include "db/sqlite.h"
#include <std/std-dbg.h>
#include <std/std-err.h>
#include <std/std-file.h>

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

typedef struct
{
  int sockfd;
  int connfd;
  struct sockaddr_in servaddr, cli;
  socklen_t len;
} conn_t;

static conn_t conn;
static db_t db;

int
handle_connection ()
{
  log_t log;
  size_t n, retry = 0;
  DEBUG_LOG ("Handling connection\n");

  while (1)
    {
      bzero (&log, sizeof (log_t));
      if (readp (conn.connfd, &log, sizeof (log_t), &n))
        {
          ERROR ("Failed to read from socket: %d\n", conn.connfd);
          continue;
        }

      if (n == 0) /* client sent empty string, or client did not close port */
        {
          if (retry == 4)
            {
              ERROR ("Max retries reached\n");
              break;
            }

          retry++;
          continue;
        }

      sqlite_local_db_log (&db, &log);
      /* sqlite_local_db_exec_query (&db, buff); */
    }
  return 0;
}

int
main ()
{
  conn.sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (conn.sockfd == -1)
    {
      ERRNO ("Failed to create socket\n");
      return 1;
    }

  if (setsockopt (conn.sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                  (const void *)&(int){ 1 }, sizeof (int))
      == -1)
    {
      ERRNO ("Failed to set socket options: %d to value: %d\n",
             SO_REUSEADDR | SO_REUSEPORT, 1);
      return 1;
    }

  bzero (&conn.servaddr, sizeof (conn.servaddr));
  conn.servaddr.sin_family = AF_INET;
  conn.servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
  conn.servaddr.sin_port = htons (PORT);

  if ((bind (conn.sockfd, (SA *)&conn.servaddr, sizeof (conn.servaddr))) != 0)
    {
      ERRNO ("Failed to bind socket\n");
      return 1;
    }

  if ((listen (conn.sockfd, 5)) != 0)
    {
      ERRNO ("Failed to listen to socket\n");
      return 1;
    }

  conn.len = sizeof (conn.cli);

  DB_INIT_SERVER (&db, "edr_server.db");

  conn.connfd = accept (conn.sockfd, (SA *)&conn.cli, &conn.len);
  if (conn.connfd < 0)
    {
      ERRNO ("Failed to accept connection\n");
      return 1;
    }

  handle_connection ();

  close (conn.sockfd);
  DB_EXIT (&db);

  return 0;
}
