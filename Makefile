OBJS     = swtch.o context.o mythread.o
OUTS     = thtest1 thtest2 thtest3 thtest4 thtest5

CC       = gcc
RM       = rm -f
CPPFLAGS =
CFLAGS   = -std=c11 -D_ISOC11_SOURCE -Wall -Wextra -Werror -pedantic-errors -O0 -g -m32 -mstackrealign
LDFLAGS  =

.PHONY: all clean
all: $(OUTS)
clean:
	$(RM) $(OUTS) $(addsuffix .o,$(OUTS)) $(OBJS)

$(OUTS): %: %.o $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
%.o: %.S
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<
%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

context.o: context.h
mythread.o: context.h mythread.h
$(addsuffix .o,$(OUTS)): mythread.h
