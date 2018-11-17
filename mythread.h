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
void start_threads();
void start_threads_dp();
#ifdef MT_DISABLE_PREEMPTION
#define start_threads start_threads_dp
#endif

// callable only in threads
void yield();
void th_exit();
void wait(void *a);
void notify(mythread_t th, void *a);
void notify_all(void *a);
void notify_any(void *a);
void atomic_begin();
void atomic_finish();

// wait for what ?
extern void *for_stdin;
extern void *for_stdout;

// alternatives for unsafe or blocking functions
int th_scanf(const char *fmt, ...);
int th_printf(const char *fmt, ...);
#ifdef MT_DISABLE_NONBLOCKING_IO
#define scanf(fmt, ...) { atomic_begin(); scanf(fmt, __VA_ARGS__); atomic_finish(); }
#define printf(fmt, ...) { atomic_begin(); printf(fmt, __VA_ARGS__); atomic_finish(); }
#else
#define scanf th_scanf
#define printf th_printf
#endif
