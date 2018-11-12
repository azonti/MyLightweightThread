#define _POSIX_SOURCE   1
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/select.h>

#include "mythread.h"



struct mythread thrds[MAXTHREADS];
int rnng = -2;
int last_notified = -1;
struct context *schdctx;

void schd() {
  int alive = 2;
  while (alive > 1) {
    alive = 0;

    for (rnng = 0; rnng < MAXTHREADS; rnng++) {
      if (thrds[rnng].state != MT_UNUSED) alive++;
      if (thrds[rnng].state != MT_READY) continue;

      thrds[rnng].state = MT_RUNNING;
      swtch(&schdctx, thrds[rnng].ctx);

      if (thrds[rnng].state == MT_ZOMBIE) {
        thrds[rnng].state = MT_UNUSED;
        free(thrds[rnng].stack);
      }
    }
  }
}



sigset_t onlyalrm;
struct timeval tv0;
void *for_stdout = "kakikomitai";

void alrm_handler(int unused) {
  yield();

  (void)unused;
}

void th_for_stdout(int unused) {
  fd_set wfds;
  FD_ZERO(&wfds);
  FD_SET(STDOUT_FILENO, &wfds);

  block_alrm();

  while (1) {
    if (select(STDOUT_FILENO+1, NULL, &wfds, NULL, &tv0) == 1) notify_any((void *)for_stdout);
    yield();
  }

  (void)unused;
}

void start_schd() {
  sigemptyset(&onlyalrm);
  sigaddset(&onlyalrm, SIGVTALRM);
  sigprocmask(SIG_BLOCK, &onlyalrm, NULL);

  struct sigaction sa;
  sa.sa_handler = alrm_handler;
  sa.sa_flags = SA_NODEFER;
  sigaction(SIGVTALRM, &sa, NULL);

  struct itimerval it;
  it.it_value.tv_sec = 0;
  it.it_value.tv_usec = 1;
  it.it_interval.tv_sec = 0;
  it.it_interval.tv_usec = 1;
  setitimer(ITIMER_VIRTUAL, &it, NULL);

  rnng++;

  tv0.tv_sec = 999999999;
  tv0.tv_usec = 0;

  start_thread(new_thread(th_for_stdout, 0));

  schd();
}



mythread_t new_thread(void (*fun)(int), int arg) {
  block_alrm();

  int i;
  for (i = 0; i < MAXTHREADS; i++) if (thrds[i].state == MT_UNUSED) break;
  if (i == MAXTHREADS) return NULL;

  thrds[i].stack = malloc(STACKSIZE);
  if (!thrds[i].stack) return NULL;
  thrds[i].state = MT_EMBRYO;
  thrds[i].ctx = new_context((char *)thrds[i].stack + STACKSIZE - 1, unblock_alrm, fun, arg, th_exit);
  thrds[i].block_dpth = 1;

  unblock_alrm();

  return &thrds[i];
}

void start_thread(mythread_t th) {
  if (rnng >= 0) block_alrm();

  if (th->state == MT_EMBRYO) th->state = MT_READY;

  if (rnng >= 0) unblock_alrm();

  if (rnng < -1) start_schd();

}

void start_threads() {
  if (rnng >= 0) block_alrm();

  for (int i = 0; i < MAXTHREADS; i++) if (thrds[i].state == MT_EMBRYO) thrds[i].state = MT_READY;

  if (rnng >= 0) unblock_alrm();

  if (rnng < -1) start_schd();
}

void yield() {
  block_alrm();

  thrds[rnng].state = MT_READY;
  swtch(&thrds[rnng].ctx, schdctx);

  unblock_alrm();
}

void th_exit() {
  block_alrm();

  thrds[rnng].state = MT_ZOMBIE;
  swtch(&thrds[rnng].ctx, schdctx);
}

void wait(void *a) {
  block_alrm();

  thrds[rnng].state = MT_SLEEPING;
  thrds[rnng].chan = a;
  swtch(&thrds[rnng].ctx, schdctx);

  unblock_alrm();
}

void notify(mythread_t thrd, void *a) {
  block_alrm();

  if (thrd->chan != a) return;
  thrd->state = MT_READY;
  thrd->chan = NULL;

  unblock_alrm();
}

void notify_all(void *a) {
  block_alrm();

  for (int i = 0; i < MAXTHREADS; i++) {
    if (thrds[i].chan != a) continue;
    thrds[i].state = MT_READY;
    thrds[i].chan = NULL;
  }

  unblock_alrm();
}

void notify_any(void *a) {
  block_alrm();

  for (int i = last_notified + 1; i < MAXTHREADS; i++) {
    if (thrds[i].chan != a) continue;
    thrds[i].state = MT_READY;
    thrds[i].chan = NULL;
    last_notified = i;
    return;
  }
  for (int i = 0; i < MAXTHREADS; i++) {
    if (thrds[i].chan != a) continue;
    thrds[i].state = MT_READY;
    thrds[i].chan = NULL;
    last_notified = i;
    return;
  }

  unblock_alrm();
}



void block_alrm() {
  sigprocmask(SIG_BLOCK, &onlyalrm, NULL);
  thrds[rnng].block_dpth++;
}

void unblock_alrm() {
  thrds[rnng].block_dpth--;
  if (thrds[rnng].block_dpth == 0) sigprocmask(SIG_UNBLOCK, &onlyalrm, NULL);
}
