#include "can.h"
#include <std/std-dbg.h>
#include <std/std-macro.h>
#include <std/std.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

can_t
can_init (char *name, can_type type)
{
  can_t can = { 0 };
  can.type = type;
  strcpy (can.name, name);

  snprintf (can.cmd, CAN_CMD_LENGTH, CAN_CMD[type], name);
  return can;
}

can_msg
can_rcv_msg (char *msg, int len)
{
  UNUSED (len);
  can_msg cmsg = { 0 };

  sscanf (msg,
          "(%" STR (CAN_TIMESTAMP_LENGTH) "[^)]) %*s %" STR (
              CAN_ID_LENGTH) "[^#]#%" STR (CAN_DATA_LENGTH) "s",
          cmsg.timestamp, cmsg.can_id, cmsg.can_data);

  return cmsg;
}

inline void
can_send_msg (can_t can, char *cmd, char *payload)
{
  snprintf (cmd, CAN_CMD_LENGTH, can.cmd, payload);
  system (cmd);
}

int
can_run_receive (can_t *can, void (*func) (char *, int, void *), void *data)
{
  char rcv_msg[CAN_MSG_LENGTH];

  if (can->stream == 0) /* when process is restarted do not reopen pipe */
    {
      int pipefd[2];
      pipe (pipefd);
      can->pid = fork ();
      if (can->pid == 0) /* child */
        {
          close (pipefd[0]);
          dup2 (pipefd[1], STDOUT_FILENO);
          dup2 (pipefd[1], STDERR_FILENO);
          char *argv[] = { can->cmd, can->name, (char *)"-L", NULL };

          if (execvp (can->cmd, argv) < 0)
            perror ("execv");
        }

      close (pipefd[1]);
      can->stream = fdopen (pipefd[0], "r");
      CAN_CHECK_STREAM ((*can));
    }

  while (fgets (rcv_msg, CAN_MSG_LENGTH, can->stream))
    {
      func (rcv_msg, CAN_MSG_LENGTH, data);
    }

  fclose (can->stream);
  can->stream = 0;

  return 0;
}

int
can_run_send (can_t can, can_frame *frame)
{
  char payload[CAN_PAYLOAD_LENGTH];
  char cmd[CAN_CMD_LENGTH];

  snprintf (payload, CAN_PAYLOAD_LENGTH, "%s#%s", frame->can_id,
            frame->can_data);
  can_send_msg (can, cmd, payload);

  return 0;
}

int
can_run_play (can_t can, char *filepath)
{
  char *argv[] = { can.cmd, (char *)"-I", filepath, NULL };

  if (execvp (can.cmd, argv) < 0)
    perror ("execv");

  return 0;
}

int
can_run (can_t *can, void *var, void *data)
{
  switch (can->type)
    {
    case can_receive:
      return can_run_receive (can, (void (*) (char *, int, void *))var, data);
      break;
    case can_send:
      return can_run_send (*can, (can_frame *)var);
      break;
    case can_play:
      return can_run_play (*can, (char *)var);
      break;
    default:
      DEBUG_ERROR ("No such can type %d", can->type);
      return 1;
    }

  return 0;
}
