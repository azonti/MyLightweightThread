struct context {
  void *edi;
  void *esi;
  void *ebx;
  void *ebp;
  void (*from)();
  void (*fun)(int);
  void (*to)();
  int arg;
};

struct context *new_context(void *btm, void (*from)(), void (*fun)(int), int arg, void (*to)());
void swtch(struct context **oldctx, struct context *newctx);
