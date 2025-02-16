#pragma once
#include <sys/time.h>
#include <signal.h>

extern struct itimerval it_val;

void pthread_sigalrm_handler (int sig);
