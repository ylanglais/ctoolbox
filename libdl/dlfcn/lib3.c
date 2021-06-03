#include <stdio.h>
#include <dlfcn.h>
void foo() {
	void (*next_foo)();
	printf("lib3.foo()\n");	
	next_foo = (void (*)()) dlsym(RTLD_NEXT, "foo");
	next_foo();
}
