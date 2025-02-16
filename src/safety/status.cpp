#include "status.hpp"
#include "data/index.hpp"
#include <std/std-dbg.h>
#include <string.h>
#include <unistd.h>

#define STATUS_LOG_INT 0.1f
#define STATUS_LOG_PM (1.0f / STATUS_LOG_INT)

#define STATUS_LOG_BEFORE_TIME 20.0f /* CHECK: value */
#define STATUS_LOG_BEFORE_SIZE (int)(STATUS_LOG_PM * STATUS_LOG_BEFORE_TIME)

#define STATUS_LOG_AFTER_TIME 30.0f /* CHECK: value */
#define STATUS_LOG_AFTER_SIZE (int)(STATUS_LOG_PM * STATUS_LOG_AFTER_TIME)

static int
safety_status_init_log (status_log_t *log, size_t size)
{
  log->indicator = 0;

  if (cbuf_mail_init (&log->speed, size))
    return 1;
  if (cbuf_mail_init (&log->brake, size))
    return 1;
  if (cbuf_mail_init (&log->acc_pedal, size))
    return 1;
  if (cbuf_mail_init (&log->str_wheel_speed, size))
    return 1;
  if (cbuf_mail_init (&log->acc_pedal, size))
    return 1;
  if (cbuf_mail_init (&log->acc_long, size))
    return 1;
  if (cbuf_mail_init (&log->acc_lat, size))
    return 1;

  if (cbuf_mail_init (&log->cusum_h_acc_long, size))
    return 1;
  if (cbuf_mail_init (&log->cusum_l_acc_long, size))
    return 1;

  if (cbuf_mail_init (&log->cusum_h_acc_lat, size))
    return 1;
  if (cbuf_mail_init (&log->cusum_l_acc_lat, size))
    return 1;

  if (cbuf_mail_init (&log->cusum_acc_long_est_std_dev, size))
    return 1;

  if (cbuf_mail_init (&log->cusum_acc_lat_est_std_dev, size))
    return 1;

  if (cbuf_mail_init (&log->cpu_time, size))
    return 1;

  return 0;
}

int
safety_status_deinit (status_log_t *log)
{
  cbuf_mail_deinit (&log->speed);
  cbuf_mail_deinit (&log->brake);
  cbuf_mail_deinit (&log->acc_pedal);
  cbuf_mail_deinit (&log->str_wheel_speed);
  cbuf_mail_deinit (&log->acc_pedal);
  cbuf_mail_deinit (&log->acc_long);
  cbuf_mail_deinit (&log->acc_lat);

  cbuf_mail_deinit (&log->cusum_h_acc_long);
  cbuf_mail_deinit (&log->cusum_l_acc_long);

  cbuf_mail_deinit (&log->cusum_h_acc_lat);
  cbuf_mail_deinit (&log->cusum_l_acc_lat);

  cbuf_mail_deinit (&log->cusum_acc_long_est_std_dev);
  cbuf_mail_deinit (&log->cusum_acc_lat_est_std_dev);

  cbuf_mail_deinit (&log->cpu_time);

  cbuf_mail_deinit (&log->cpu_time);

  return 0;
}

int
safety_status_reset_log (status_log_t *log)
{
  log->indicator = 0;

  cbuf_mail_reset (&log->speed);
  cbuf_mail_reset (&log->brake);
  cbuf_mail_reset (&log->acc_pedal);
  cbuf_mail_reset (&log->str_wheel_speed);
  cbuf_mail_reset (&log->acc_pedal);
  cbuf_mail_reset (&log->acc_long);
  cbuf_mail_reset (&log->acc_lat);

  cbuf_mail_reset (&log->cusum_h_acc_long);
  cbuf_mail_reset (&log->cusum_l_acc_long);

  cbuf_mail_reset (&log->cusum_h_acc_lat);
  cbuf_mail_reset (&log->cusum_l_acc_lat);

  cbuf_mail_reset (&log->cusum_acc_long_est_std_dev);
  cbuf_mail_reset (&log->cusum_acc_lat_est_std_dev);

  cbuf_mail_reset (&log->cpu_time);

  return 0;
}

static inline void
safety_status_cusum (cusum_t *cus)
{
  cus->shift_detect = 1.0f;
  cus->est_std_dev = 0.0f;
  cus->h = 5.0f;
}

int
safety_status_init (safety_status *status)
{
  memset (status, 0, sizeof (*status));
  cpu_upd (&status->cpu);

  safety_status_cusum (&status->cus_lat);
  safety_status_cusum (&status->cus_long);

  if (safety_status_init_log (&status->log_before, STATUS_LOG_BEFORE_SIZE))
    return 1;

  return 0;
}

#define STARTED_MSG "Crash Started"
#define MID_MSG "Crash Mid"
#define ENDED_MSG "Crash Ended"
static mail_t mail_tmp,
    notif_mail = { .priority = 1,
                   .type = MT_STORAGE,
                   .data = { .type = DATA_LOG,
                             .data = { .log = { .type = LOG_EVENT_CRASH_START,
                                                .data = { .log = "" },
                                                .db_time = {} } } },
                   .info = "" },
    data_log
    = { .priority = 1,
        .type = MT_STORAGE,
        .data = { .type = DATA_LOG,
                  .data = { .log = { .type = LOG_DATA,
                                     .data = { .data = { .type = DB_DATA_FLOAT,
                                                         .data = { .f = 0.0f },
                                                         .pid = 0 } },
                                     .db_time = {} } } },
        .info = "" },
    cusum_mail
    = { .priority = 1,
        .type = MT_STORAGE,
        .data = { .type = DATA_LOG,
                  .data = { .log = { .type = LOG_DATA,
                                     .data = { .data = { .type = DB_DATA_FLOAT,
                                                         .data = { .f = 0.0f },
                                                         .pid = 100 } },
                                     .db_time = {} } } },
        .info = "" };
void
safety_status_update (safety_status *status, mail_t *in)
{
  data_log.data.data.log.data.data.pid = (int)*in->info;
  data_log.data.data.log.data.data.data.f = in->data.data.f;

  switch ((can_data_t)*in->info)
    {
    case CAN_DATA_BRAKE_SWITCH_VALUE:
      cbuf_mail_write (&status->log_before.brake, &data_log);
      break;

    case CAN_DATA_ACCELERATOR_PEDAL_VALUE:
      cbuf_mail_write (&status->log_before.acc_pedal, &data_log);
      break;

    case CAN_DATA_VEHICLE_SPEED:
      cbuf_mail_write (&status->log_before.speed, &data_log);

      cpu_upd (&status->cpu);
      cusum_mail.data.data.log.data.data.pid = 200;
      cusum_mail.data.data.log.data.data.data.f = status->cpu.usage;
      cbuf_mail_write (&status->log_before.cpu_time, &cusum_mail);
      break;

    case CAN_DATA_ACCELERATION_SPEED_LONGITUDINAL:
      cbuf_mail_write (&status->log_before.acc_long, &data_log);
      status->sft_resp = cusum (&status->cus_long, in->data.data.f);

      cusum_mail.data.data.log.data.data.pid = 100;
      cusum_mail.data.data.log.data.data.data.f = status->cus_long.Sh_prev;
      cbuf_mail_write (&status->log_before.cusum_h_acc_long, &cusum_mail);

      cusum_mail.data.data.log.data.data.pid = 101;
      cusum_mail.data.data.log.data.data.data.f = status->cus_long.Sl_prev;
      cbuf_mail_write (&status->log_before.cusum_l_acc_long, &cusum_mail);

      cusum_mail.data.data.log.data.data.pid = 102;
      cusum_mail.data.data.log.data.data.data.f = status->cus_long.est_std_dev;
      cbuf_mail_write (&status->log_before.cusum_acc_long_est_std_dev,
                       &cusum_mail);

      if (status->sft_resp == 1 && status->crash_status == 0)
        {
          status->crash_status = 1;
          status->mean_i = &status->cus_long.mean_i;
        }
      break;

    case CAN_DATA_ACCELERATION_SPEED_LATERAL:
      cbuf_mail_write (&status->log_before.acc_lat, &data_log);
      // DEBUG_LOG ("ACCELERATION_SPEED_LATERAL %f\n", in->data.data.f);
      // printf ("ACC LAT: %f\n", in->data.data.f);
      status->sft_resp = cusum (&status->cus_lat, in->data.data.f);

      cusum_mail.data.data.log.data.data.pid = 103;
      cusum_mail.data.data.log.data.data.data.f = status->cus_lat.Sh_prev;
      cbuf_mail_write (&status->log_before.cusum_h_acc_lat, &cusum_mail);

      cusum_mail.data.data.log.data.data.pid = 104;
      cusum_mail.data.data.log.data.data.data.f = status->cus_lat.Sl_prev;
      cbuf_mail_write (&status->log_before.cusum_l_acc_lat, &cusum_mail);

      cusum_mail.data.data.log.data.data.pid = 105;
      cusum_mail.data.data.log.data.data.data.f = status->cus_lat.est_std_dev;
      cbuf_mail_write (&status->log_before.cusum_acc_lat_est_std_dev,
                       &cusum_mail);

      if (status->sft_resp == 1 && status->crash_status == 0)
        {
          status->crash_status = 1;
          status->mean_i = &status->cus_lat.mean_i;
        }
      break;

    case CAN_DATA_UNKNOWN:
    default:
      DEBUG_LOG ("Unknown pid: %d,\n", (int)*in->info);
    }
}

#define SAFETY_STATUS_LOG(type)                                               \
  if (status->log_before.type##_i == STATUS_LOG_AFTER_SIZE)                   \
    {                                                                         \
      status->log_before.indicator++;                                         \
    }                                                                         \
  else if (status->log_before.type##_i < STATUS_LOG_AFTER_SIZE)               \
    {                                                                         \
      data_log.data.data.log.data.data.pid = (int)*in->info;                  \
      data_log.data.data.log.data.data.data.f = in->data.data.f;              \
      db_get_time (&data_log.data.data.log.db_time);                          \
      STATUS_SEND (mailbox, data_log);                                        \
    }                                                                         \
  status->log_before.type##_i++;

#define SAFETY_STATUS_LOG_T(type, cus, pidv)                                  \
  if (status->log_before.type##_i == STATUS_LOG_AFTER_SIZE)                   \
    {                                                                         \
      status->log_before.indicator++;                                         \
    }                                                                         \
  else if (status->log_before.type##_i < STATUS_LOG_AFTER_SIZE)               \
    {                                                                         \
      data_log.data.data.log.data.data.pid = pidv;                            \
      data_log.data.data.log.data.data.data.f = cus;                          \
      db_get_time (&data_log.data.data.log.db_time);                          \
      STATUS_SEND (mailbox, data_log);                                        \
    }                                                                         \
  status->log_before.type##_i++;

#define SAFETY_STATUS_LOG_BUFFER(type)                                        \
  cbuf_mail_move_index (&status->log_before.type,                             \
                        STATUS_LOG_BEFORE_SIZE + 1);                          \
  for (size_t i = 0; i < status->log_before.type.size; i++)                   \
    {                                                                         \
      cbuf_mail_read_ahead (&status->log_before.type, &mail_tmp);             \
      memcpy (&mail_tmp.data.data.log.db_time,                                \
              &data_log.data.data.log.db_time, sizeof (db_time_t));           \
      STATUS_SEND (mailbox, mail_tmp);                                        \
    }

static db_time_t db_time;
void
safety_status_log (safety_status *status, postbox_single_t *mailbox, mail_t *in)
{
  if (status->crash_status == 0)
    return;
  else if (status->crash_status == 1)
    {
      db_get_time (&db_time);
      notif_mail.data.data.log.type = LOG_EVENT_CRASH_START;
      snprintf (notif_mail.data.data.log.data.log, LOG_DATA_SIZE,
                STARTED_MSG " %ld %f %f %s", *status->mean_i,
                status->cus_long.h, status->cus_lat.h, db_time.timestamp);
      STATUS_SEND (mailbox, notif_mail);

      db_get_time (&data_log.data.data.log.db_time);

      DEBUG_LOG ("TRANSACTION_COUNT before: %ld\n", TRANSACTION_COUNT);
      SAFETY_STATUS_LOG_BUFFER (brake)
      SAFETY_STATUS_LOG_BUFFER (acc_pedal)
      SAFETY_STATUS_LOG_BUFFER (speed)
      SAFETY_STATUS_LOG_BUFFER (acc_long)
      SAFETY_STATUS_LOG_BUFFER (acc_lat)

      SAFETY_STATUS_LOG_BUFFER (cusum_h_acc_long)
      SAFETY_STATUS_LOG_BUFFER (cusum_l_acc_long)
      SAFETY_STATUS_LOG_BUFFER (cusum_h_acc_lat)
      SAFETY_STATUS_LOG_BUFFER (cusum_l_acc_lat)

      SAFETY_STATUS_LOG_BUFFER (cusum_acc_lat_est_std_dev);
      SAFETY_STATUS_LOG_BUFFER (cpu_time);
      SAFETY_STATUS_LOG_BUFFER (cusum_acc_long_est_std_dev);

      notif_mail.data.data.log.type = LOG_EVENT_CRASH_MID;
      snprintf (notif_mail.data.data.log.data.log, LOG_DATA_SIZE,
                MID_MSG " %ld", *status->mean_i);
      STATUS_SEND (mailbox, notif_mail);

      status->crash_status++;
      return;
    }

  switch ((can_data_t)*in->info)
    {
    case CAN_DATA_BRAKE_SWITCH_VALUE:
      SAFETY_STATUS_LOG (brake);
      break;

    case CAN_DATA_ACCELERATOR_PEDAL_VALUE:
      SAFETY_STATUS_LOG (acc_pedal);
      break;

    case CAN_DATA_VEHICLE_SPEED:
      SAFETY_STATUS_LOG (speed);
      SAFETY_STATUS_LOG_T (cpu_time, status->cpu.usage, 200);
      break;

    case CAN_DATA_ACCELERATION_SPEED_LONGITUDINAL:
      SAFETY_STATUS_LOG (acc_long);
      SAFETY_STATUS_LOG_T (cusum_h_acc_long, status->cus_long.Sh_prev, 100)
      SAFETY_STATUS_LOG_T (cusum_l_acc_long, status->cus_long.Sl_prev, 101)
      SAFETY_STATUS_LOG_T (cusum_acc_long_est_std_dev,
                           status->cus_long.est_std_dev, 102);
      break;

    case CAN_DATA_ACCELERATION_SPEED_LATERAL:
      SAFETY_STATUS_LOG (acc_lat);
      SAFETY_STATUS_LOG_T (cusum_h_acc_lat, status->cus_lat.Sh_prev, 103)
      SAFETY_STATUS_LOG_T (cusum_l_acc_lat, status->cus_lat.Sl_prev, 104)
      SAFETY_STATUS_LOG_T (cusum_acc_lat_est_std_dev,
                           status->cus_long.est_std_dev, 105);
      break;

    case CAN_DATA_UNKNOWN:
    default:
      DEBUG_LOG ("Unknown pid: %d,\n", (int)*in->info);
    }

  if (status->log_before.indicator
      >= CAN_DATA_SIZE - 1
             + 7) // CAN_DATA_SIZE -1 + extra cusum and std dev buffers
    {
      notif_mail.data.data.log.type = LOG_EVENT_CRASH_END;
      db_get_time (&db_time);
      snprintf (notif_mail.data.data.log.data.log, LOG_DATA_SIZE,
                ENDED_MSG " %ld", *status->mean_i);
      STATUS_SEND (mailbox, notif_mail);

      status->crash_status = 0;
      status->log_before.indicator = 0;
      DEBUG_LOG ("Crash ended\n");
    }
}

// TODO: remove
// void
// get_consuption (float *consumption)
// {
//   FILE *fp;
//   fp = fopen ("/sys/class/power_supply/BAT0/power_now", "r");
//   if (fp == NULL)
//     {
//       perror ("fopen");
//       return;
//     }
//   fscanf (fp, "%f", consumption);
//   fclose (fp);
// }

#define CPU_N 50
void
cpu_upd (cpu_time_t *cpu)
{
  if (cpu->times++ > 1)
    {
      if (cpu->times >= 2 + CPU_N)
        cpu->times = 1;
      return;
    }

  FILE *fp;
  long user, nice, system, idle, iowait, irq, softirq;

  fp = fopen ("/proc/stat", "r");
  if (fp == NULL)
    {
      perror ("fopen");
      return;
    }
  fscanf (fp, "cpu %ld %ld %ld %ld %ld %ld %ld", &user, &nice, &system, &idle,
          &iowait, &irq, &softirq);
  fclose (fp);

  if (cpu->calcp == 0) // prime
    {
      cpu->ttime = user + nice + system + idle + irq + softirq;
      cpu->itime = idle;
      cpu->calcp = 1;
    }
  else
    {
      const long total_time = user + nice + system + idle + irq + softirq,
                 total_idle = idle;
      const long total_time_diff = total_time - cpu->ttime;
      const long idle_time_diff = total_idle - cpu->itime;

      cpu->usage = 100.0 * (double)(total_time_diff - idle_time_diff)
                   / (double)total_time_diff;
      cpu->ttime = total_time;
      cpu->itime = total_idle;
    }
}
