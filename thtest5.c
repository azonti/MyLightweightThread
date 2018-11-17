#include <stdio.h>
#include "mythread.h"

char input[256];

void foo(int c) {
    while (1) {
        printf("foo : %d\n", c);
        c += 1;
    }
}

void bar(int c) {
    while (1) {
        scanf("%s", input);
    }
    (void)c;
}

void baz(int c) {
    while (1) {
        printf("%s\n", input);
    }
    (void)c;
}

int main() {
    new_thread(foo, 0);
    new_thread(bar, 0);
    new_thread(baz, 0);
    start_threads();
    return 0;
}
