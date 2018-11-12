#include "context.h"

struct context *new_context(void *btm, void (*from)(), void (*fun)(int), int arg, void (*to)()) {
  struct context *ctx = (struct context *)((char *)btm - sizeof(struct context) + 1);
  ctx->ebp = &ctx->from;
  ctx->from = from;
  ctx->fun = fun;
  ctx->to = to;
  ctx->arg = arg;
  return ctx;
}
