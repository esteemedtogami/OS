#ifndef A4_HPP
#define A4_HPP
//#define _GNU_SOURCE
#define NUMFLAVORS 4
#define NUMSLOTS 50000
#define NUMCONSUMERS 50
#define NUMPRODUCERS 30
#include <iostream>
#include <sched.h>
#include <utmpx.h>
#include <cstdlib>
#include <string>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include <strings.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

typedef struct
{
  int flavor[NUMFLAVORS][NUMSLOTS];
  int outptr[NUMFLAVORS];
  int in_ptr[NUMFLAVORS];
  int serial[NUMFLAVORS];
  int spaces[NUMFLAVORS];
  int donuts[NUMFLAVORS];
  
} DONUT_SHOP;

void *sig_waiter(void *arg);
void *producer(void *arg);
void *consumer(void *arg);
void sig_handler(int);
#endif
