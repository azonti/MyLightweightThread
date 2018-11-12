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
  int block_dpth;
} *mythread_t;

mythread_t new_thread(void (*fun)(int), int arg);
void start_thread(mythread_t th);
void start_threads();
void yield();
void th_exit();
void wait(void *a);
void notify(mythread_t th, void *a);
void notify_all(void *a);
void notify_any(void *a);

// wait for what ?
extern void *for_stdout;

// critical functions
void block_alrm();
void unblock_alrm();
#define printf(fmt, ...) { block_alrm(); printf(fmt, __VA_ARGS__); unblock_alrm(); }
