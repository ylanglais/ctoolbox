#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
int
main(void) {
	void *h;
	void (*func)() = NULL;
	if (!(h = dlopen("libhelloworld.so", RTLD_LAZY))) {
		perror("cannot open libhelloworld.so");
		return 1;
	}
	if ((func = dlsym(h, "helloworld"))) func();
	else perror("cannot resolve helloworld");
	dlclose(h);
	return 0;
}
