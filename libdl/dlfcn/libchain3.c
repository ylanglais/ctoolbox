#include <stdio.h>
#include <dlfcn.h>
void foo() {
        void (*next_foo)();
        printf("libchain3.foo()\n");
        next_foo = (void (*)()) dlsym(RTLD_NEXT, "foo");
        printf("next_foo = %x\n", next_foo);
        if (next_foo) next_foo();
} 
