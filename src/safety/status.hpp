#pragma once
#include "data/cbuf_mail.h"
#include "data/utils.h"
#include "db/db_utils.h"
#include "global.hpp"
#include "mail/mail.hpp"

typedef union
{
  postbox_single_t *single;
} status_box_t;

#define STATUS_SEND(msgbox, mail) MAIL_SEND_SINGLE (msgbox, mail)

typedef struct
{
  long ttime;
  long itime;

  int calcp;
  double usage;
  size_t times;
} cpu_time_t;

typedef struct
{
  cbuf_mail_t speed;
  size_t speed_i;
  cbuf_mail_t brake;
  size_t brake_i;
  cbuf_mail_t acc_pedal;
  size_t acc_pedal_i;
  cbuf_mail_t str_wheel_speed;
  size_t str_wheel_speed_i;
  cbuf_mail_t gear;
  size_t gear_i;
  cbuf_mail_t acc_long;
  size_t acc_long_i;
  cbuf_mail_t acc_lat;
  size_t acc_lat_i;

  cbuf_mail_t cusum_h_acc_long;
  size_t cusum_h_acc_long_i;
  cbuf_mail_t cusum_h_acc_lat;
  size_t cusum_h_acc_lat_i;

  cbuf_mail_t cusum_l_acc_long;
  size_t cusum_l_acc_long_i;
  cbuf_mail_t cusum_l_acc_lat;
  size_t cusum_l_acc_lat_i;

  cbuf_mail_t cusum_acc_lat_est_std_dev;
  size_t cusum_acc_lat_est_std_dev_i;

  cbuf_mail_t cusum_acc_long_est_std_dev;
  size_t cusum_acc_long_est_std_dev_i;

  cbuf_mail_t cpu_time;
  size_t cpu_time_i;

  size_t indicator;
} status_log_t;

typedef struct
{
  int brake;
  cusum_t cus_long;
  cusum_t cus_lat;
  size_t *mean_i;
  int sft_resp;
  int crash_status;

  cpu_time_t cpu;

  status_log_t log_before;
  size_t log_size;
} safety_status;

int safety_status_init (safety_status *status);
void safety_status_update (safety_status *status, mail_t *in);
void safety_status_log (safety_status *status, postbox_single_t *mailbox,
                        mail_t *in);

void get_consuption (float *consumption);
void cpu_upd (cpu_time_t *cpu);
