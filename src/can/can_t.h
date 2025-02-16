#pragma once
#include <stdio.h>

#if defined(__cplusplus)
extern "C"
{
#endif

#define CAN_PAYLOAD_LENGTH 24
#define CAN_FILENAME_LENGTH 200
extern char *CAN_CMD[];

typedef enum
{
  can_receive,
  can_play,
  can_send,
} can_type;

#define CAN_NAME_LENGTH 7
#define CAN_CMD_LENGTH 50

typedef struct
{
  char cmd[CAN_CMD_LENGTH];
  char name[CAN_NAME_LENGTH];
  can_type type;
  FILE *stream;
  int pid;
} can_t;

#define CAN_MSG_LENGTH 50
#define CAN_ID_LENGTH 4
#define CAN_DATA_LENGTH 17
#define CAN_HASH_LENGTH 8
#define CAN_TIMESTAMP_LENGTH 18
#define FLOAT_LENGTH 8
#define FLOAT_LENGTH_2 4

typedef struct
{
  char can_id[CAN_ID_LENGTH];
  char can_data[CAN_DATA_LENGTH];
  char timestamp[CAN_TIMESTAMP_LENGTH];
} can_msg;

typedef struct
{
  char can_id[CAN_ID_LENGTH];
  char can_data[CAN_DATA_LENGTH];
} can_frame;
#if defined(__cplusplus)
}
#endif
