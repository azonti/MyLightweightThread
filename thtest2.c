#include <stdio.h>
#define MT_DISABLE_PREEMPTION
#define MT_DISABLE_NONBLOCKING_IO
#include "mythread.h"

void foo(int c) {
    while (c < 100) {
        printf("foo : %d\n", c);
        c += 1;
        yield();
    }
}

void bar(int c) {
    while (c < 100) {
        printf("bar : %d\n", c);
        yield();
        c += 2;
    }
}

void baz(int c) {
    while (c < 100) {
        printf("baz : %d\n", c);
        yield();
        c += 3;
        printf("baz : %d\n", c);
        yield();
        c += 3;
    }
}

int main() {
    new_thread(foo, 0);
    new_thread(bar, 0);
    new_thread(baz, 0);
    start_threads();
    return 0;
}
