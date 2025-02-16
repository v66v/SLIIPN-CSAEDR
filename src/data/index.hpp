#pragma once
#include <std/std-macro.h>

#define CAN_DATA_ENUM_VARIANT(NAME, VALUE) CAN_DATA_##NAME = (VALUE),
#define CAN_DATA_ENUM(VARIANT)                                                \
  VARIANT (UNKNOWN, 0)                                                        \
  /* VARIANT (STEERING_WHEEL_SPEED, 1) */                                     \
  VARIANT (ACCELERATOR_PEDAL_VALUE, 2)                                        \
  VARIANT (BRAKE_SWITCH_VALUE, 3)                                             \
  /* VARIANT (GEAR_SELECTION, 4)  */                                          \
  VARIANT (VEHICLE_SPEED, 5)                                                  \
  VARIANT (ACCELERATION_SPEED_LONGITUDINAL, 6)                                \
  VARIANT (ACCELERATION_SPEED_LATERAL, 7)

#define PLUS_ONE(...) +1
#define CAN_DATA_COUNT (0 CAN_DATA_ENUM (PLUS_ONE))
#define CAN_DATA_SIZE CAN_DATA_COUNT

typedef enum
{
  CAN_DATA_ENUM (CAN_DATA_ENUM_VARIANT)
} can_data_t;

extern const char *can_data_table[];
