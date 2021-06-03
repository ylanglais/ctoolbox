#include <stdio.h>
#include <dlfcn.h>
int main(void) {
	void *h;
	void *p;
	int  (*get)();
	int	 (*set)();
	void  (*print)();
	
	if (!(h = dlopen("libtest2.so", RTLD_LAZY))) {
		fprintf(stderr, "error loading libtest2.so: %s\n", dlerror());
		return 1;
	}
	if (!(get = (int (*)()) dlsym(h, "static_int_get"))) 
		fprintf(stderr, "error resolving static_int: %s\n", dlerror());
	else 
		printf("static_int_get() returns %d\n", get());

	if (!(set = (int (*)(int)) dlsym(h, "static_int_set"))) 
		fprintf(stderr, "error resolving static_int_set: %s\n", dlerror());
	else
		printf("setting static_int with static_int_get() to 33\n", set(33));

	if (!(print = (void (*)()) dlsym(h, "static_int_print"))) 
		fprintf(stderr, "error resolving static_int_print: %s\n", dlerror());
	else {
		printf("call static_int_print() : ");
		print();
	}
	dlclose(h);
	return 0;
}	
