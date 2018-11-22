# Context switch
#
#   void swtch(struct context **oldctx, struct context *newctx);
# 
# Save current register context in old
# and then load register context from new.

#if defined(__APPLE__) && defined(__MACH__)
.globl _swtch
_swtch:
#else
.globl swtch
swtch:
#endif
  movl 4(%esp), %eax
  movl 8(%esp), %edx

  # Save old callee-save registers
  pushl %ebp
  pushl %ebx
  pushl %esi
  pushl %edi

  # Switch stacks
  movl %esp, (%eax)
  movl %edx, %esp

  # Load new callee-save registers
  popl %edi
  popl %esi
  popl %ebx
  popl %ebp
  ret
