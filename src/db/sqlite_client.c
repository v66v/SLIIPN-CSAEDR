#include "sqlite_client.h"
#include "sqlite_base.h"
#include <std/std-dbg.h>
#include <std/std-err.h>
#include <std/std-file.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

int
sqlite_remote_db_log (db_t *db, log_t *log)
{
  size_t wrote = 0;

  if (writep (db->sockfd, (void *)log, sizeof (log_t), &wrote))
    {
      ERROR ("Failed to write to socket: %d\n", db->sockfd);
      return 1;
    }

  return 0;
}

int
sqlite_remote_db_init (db_t *db)
{
  if ((db->sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      ERRNO ("Failed to create socket\n");
      return 1;
    }

  bzero (&db->servaddr, sizeof (db->servaddr));

  db->servaddr.sin_family = AF_INET;
  db->servaddr.sin_addr.s_addr = inet_addr ("127.0.0.1");
  db->servaddr.sin_port = htons (PORT);

  if (connect (db->sockfd, (SA *)&db->servaddr, sizeof (db->servaddr)) != 0)
    {
      ERRNO ("Failed to connect to server\n");
      return 1;
    }

  /* return sqlite_remote_db_init_tables (db); */
  return 0;
}

inline void
sqlite_remote_db_exit (db_t *db)
{
  close (db->sockfd);
}
