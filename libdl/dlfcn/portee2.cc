#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>

int main(void) {
	void *h;
	void *p;

	if (!(h = dlopen("libtest.so", RTLD_LAZY))) {
		fprintf(stderr, "error loading libtest.so: %s\n", dlerror());
		return 1;
	}
	if (!(p = dlsym(h, "static_int"))) 
		fprintf(stderr, "error resolving static_int: %s\n", dlerror());
	else 
		printf("libtest.static_int is at %x and its value is %d\n", p, *(int *) p);

	if (!(p = dlsym(h, "extern_int"))) 
		fprintf(stderr, "error resolving extern_int: %s\n", dlerror());
	else
		printf("libtest.extern_int is at %x and its value is %d\n", p, *(int *) p);

	if (!(p = dlsym(h, "static_function"))) 
		fprintf(stderr, "error resolving static_function: %s\n", dlerror());
	else 
		printf("libtest.static_function is at %x\n", p);

	if (!(p = dlsym(h, "extern_function"))) 
		fprintf(stderr, "error resolving extern_function: %s\n", dlerror());
	else 
		printf("libtest.error_function is at %x\n", p);


	 pause();
	
	dlclose(h);


	return 0;
}

