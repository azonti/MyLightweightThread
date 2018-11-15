#define _POSIX_SOURCE   1
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/select.h>

#include "mythread.h"

struct mythread thrds[MAXTHREADS];
int rnng = 0;
int last_notified = -1;
struct context *schdctx;
sigset_t onlyalrm;
void *for_stdin  = "yomikomitai";
void *for_stdout = "kakikomitai";

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

void alrm_handler(int unused) {
  yield();
  (void)unused;
}

void notifier(int unused) {
  int mfd = STDIN_FILENO;
  if (mfd < STDOUT_FILENO) mfd = STDOUT_FILENO;

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  fd_set rfds, wfds;

  atomic_begin();
  while (1) {
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    FD_ZERO(&wfds);
    FD_SET(STDOUT_FILENO, &wfds);

    if (select(mfd+1, &rfds, &wfds, NULL, &tv) > 0) {
      if (FD_ISSET(STDIN_FILENO, &rfds)) notify_any((void *)for_stdin);
      if (FD_ISSET(STDOUT_FILENO, &wfds)) notify_any((void *)for_stdout);
    }

    yield();
  }

  (void)unused;
}



mythread_t new_thread(void (*fun)(int), int arg) {
  int i;
  for (i = 0; i < MAXTHREADS; i++) if (thrds[i].state == MT_UNUSED) break;
  if (i == MAXTHREADS) return NULL;

  thrds[i].stack = malloc(STACKSIZE);
  if (!thrds[i].stack) return NULL;
  thrds[i].state = MT_EMBRYO;
  thrds[i].ctx = new_context((char *)thrds[i].stack + STACKSIZE - 1, atomic_finish, fun, arg, th_exit);
  thrds[i].atomic_dpth = 1;

  return &thrds[i];
}

void start_threads() {
  new_thread(notifier, 0);

  for (int i = 0; i < MAXTHREADS; i++) if (thrds[i].state == MT_EMBRYO) thrds[i].state = MT_READY;

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

  schd();
}



void yield() {
  atomic_begin();
  thrds[rnng].state = MT_READY;
  swtch(&thrds[rnng].ctx, schdctx);
  atomic_finish();
}

void th_exit() {
  atomic_begin();
  thrds[rnng].state = MT_ZOMBIE;
  swtch(&thrds[rnng].ctx, schdctx);
}

void wait(void *a) {
  atomic_begin();
  thrds[rnng].state = MT_SLEEPING;
  thrds[rnng].chan = a;
  swtch(&thrds[rnng].ctx, schdctx);
  atomic_finish();
}

void notify(mythread_t thrd, void *a) {
  atomic_begin();
  if (thrd->chan != a) return;
  thrd->state = MT_READY;
  thrd->chan = NULL;
  atomic_finish();
}

void notify_all(void *a) {
  atomic_begin();
  for (int i = 0; i < MAXTHREADS; i++) {
    if (thrds[i].chan != a) continue;
    thrds[i].state = MT_READY;
    thrds[i].chan = NULL;
  }
  atomic_finish();
}

void notify_any(void *a) {
  atomic_begin();
  for (int i = last_notified + 1; i < MAXTHREADS; i++) {
    if (thrds[i].chan != a) continue;
    thrds[i].state = MT_READY;
    thrds[i].chan = NULL;
    last_notified = i;
    goto BREAK;
  }
  for (int i = 0; i < MAXTHREADS; i++) {
    if (thrds[i].chan != a) continue;
    thrds[i].state = MT_READY;
    thrds[i].chan = NULL;
    last_notified = i;
    goto BREAK;
  }
BREAK:
  atomic_finish();
}

void atomic_begin() {
  sigprocmask(SIG_BLOCK, &onlyalrm, NULL);
  thrds[rnng].atomic_dpth++;
}

void atomic_finish() {
  thrds[rnng].atomic_dpth--;
  if (thrds[rnng].atomic_dpth == 0) sigprocmask(SIG_UNBLOCK, &onlyalrm, NULL);
}

int th_scanf(const char *fmt, ...) {
  va_list ap;
  char str[256];
  va_start(ap, fmt);
  wait(for_stdin);
  read(STDIN_FILENO, &str, 256);
  int ret = vsscanf(str, fmt, ap);
  va_end(ap);
  return ret;
}

int th_printf(const char *fmt, ...) {
  va_list ap;
  char str[256];
  va_start(ap, fmt);
  int ret = vsprintf(str, fmt, ap);
  wait(for_stdout);
  write(STDOUT_FILENO, &str, ret);
  va_end(ap);
  return ret;
}
