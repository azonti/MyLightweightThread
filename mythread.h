#include "context.h"

#define MAXTHREADS (4)
#define STACKSIZE  (64 * 1024)

enum state {
  MT_UNUSED = 0,
  MT_EMBRYO,
  MT_READY,
  MT_RUNNING,
  MT_SLEEPING,
  MT_ZOMBIE
};

typedef struct mythread {
  enum state state;
  void *stack;
  struct context *ctx;
  void *chan;
  int atomic_dpth;
} *mythread_t;

// callable only before starting threads
mythread_t new_thread(void (*fun)(int), int arg);
#ifdef MT_DISABLE_PREEMPTION
void start_threads_dp();
#define start_threads start_threads_dp
#else
void start_threads();
#endif

// callable only in threads
void yield();
void th_exit();
#define wait th_wait // there is wait() in POSIX.1-2008 @_@
void th_wait(void *a);
void notify(mythread_t th, void *a);
void notify_all(void *a);
void notify_any(void *a);
void atomic_begin();
void atomic_finish();

// wait for what ?
extern void *for_stdin;
extern void *for_stdout;

// alternatives for unsafe or blocking functions
#ifdef MT_DISABLE_NONBLOCKING_IO
int th_scanf_dnb(const char *fmt, ...);
int th_printf_dnb(const char *fmt, ...);
#define scanf th_scanf_dnb
#define printf th_printf_dnb
#else
int th_scanf(const char *fmt, ...);
int th_printf(const char *fmt, ...);
#define scanf th_scanf
#define printf th_printf
#endif
