/*test */

#include <stdlib.h>
#include <dlfcn.h> 
#include <stdio.h>

void *opl(char *lib, int opts) {
	void *l;
	if (!(l = dlopen(lib, opts))) {
		printf("Error: %s", dlerror());
		exit(1);
	}
	return l;
}


int main() {
	void *l1, *l2, *l3, *l4;
	void (*bar)();
	l1 = opl("libchain1.so", RTLD_NOW | RTLD_GLOBAL);
	l2 = opl("libchain2.so", RTLD_NOW | RTLD_GLOBAL);
	l3 = opl("libchain3.so", RTLD_NOW | RTLD_GLOBAL);
	l4 = opl("libchain4.so", RTLD_NOW | RTLD_GLOBAL);
	bar = (void (*)()) dlsym(RTLD_DEFAULT, "foo");
	bar();
	dlclose(l4);
	dlclose(l3);
	dlclose(l2);
	dlclose(l1);
	return 0;
}
