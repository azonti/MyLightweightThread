/*
 * swtest
 * Simple examples for understanding the usage of xv6 swtch
 * Copyright (C) 2017 Takuo Watanabe
 */

#include <stdio.h>

#include "mythread.h"

void foo(int c) {
    while (1) {
        printf("foo : %d\n", c);
        c += 1;
        yield();
    }
}

void bar(int c) {
    while (1) {
        printf("bar : %d\n", c);
        yield();
        c += 2;
    }
}

void baz(int c) {
    while (1) {
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
