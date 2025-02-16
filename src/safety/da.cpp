#include "da.hpp"
#include "data/index.hpp"
#include <string.h>

static mail_t mail_notif_speed
    = { .priority = 1,
        .type = MT_ASSIST,
        .data
        = { .type = DATA_MSG,
            .data = { .msg = "Caution: major shift in speed detected" } },
        .info = "NOTIF" },
    mail_acc_pedal
    = { .priority = 1,
        .type = MT_ASSIST,
        .data
        = { .type = DATA_MSG,
            .data = { .msg = "Caution: excessive use of the accelator" } },
        .info = "NOTIF" },
    mail_notif_brake
    = { .priority = 1,
        .type = MT_ASSIST,
        .data = { .type = DATA_MSG,
                  .data = { .msg = "Caution: breaking hard frequently" } },
        .info = "NOTIF" },
    mail_assist
    = { .priority = 1,
        .type = MT_ASSIST,
        .data = { .type = DATA_MSG, .data = { .msg = "Medical attention" } },
        .info = "ASSIST" };

static int breakp = 0;
static int break_count = -1;
static int speed_shift = 0;

static size_t acc_pedal = 0;
static int acc_pedal_toggle = 0;

void
safety_da_update (safety_status *status, postbox_t *msgbox, mail_t *in)
{
  switch ((can_data_t)*in->info)
    {
    case CAN_DATA_BRAKE_SWITCH_VALUE:
      if ((int)in->data.data.f != 0) // bracking
        {
          breakp = 1;
          if (status->brake >= 5)
            {
              status->brake = 0;
              MAIL_SEND (msgbox, mail_notif_brake);
            }
        }
      else if (breakp == 1)
        {
          breakp = 0;
          break_count = 40;
          status->brake++;
        }
      else
        {
          if (break_count-- == 0)
            {
              status->brake--;
            }
        }
      break;

    case CAN_DATA_ACCELERATOR_PEDAL_VALUE:
      if (in->data.data.f > 30 && ++acc_pedal > 200 && acc_pedal_toggle == 0)
        {
          MAIL_SEND (msgbox, mail_acc_pedal);
          acc_pedal_toggle = 1;
        }
      else if (acc_pedal > 20)
        {
          acc_pedal -= 20;
          acc_pedal_toggle = 0;
        }
      break;

    case CAN_DATA_VEHICLE_SPEED:
      break;

    case CAN_DATA_ACCELERATION_SPEED_LONGITUDINAL:
    case CAN_DATA_ACCELERATION_SPEED_LATERAL:
      if (status->crash_status == 1 && speed_shift == 0)
        {
          speed_shift = 1;
          MAIL_SEND (msgbox, mail_notif_speed);
          MAIL_SEND (msgbox, mail_assist);
        }
      else
        speed_shift = 0;

      break;

    case CAN_DATA_UNKNOWN:
      break;

    default:
      break;
    }
}
