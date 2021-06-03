#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
int main(int n, char *a[]) {
	int i;
	void *lib;
	double (*sin)(double);

	if (!(lib = dlopen("/usr/lib/libm.so", RTLD_LAZY))) return 1;
	if (!(sin = (double (*)(double)) dlsym(lib, "sin"))) return 2;	

	for (i = 1; i < n; i++) printf("sin(%f) = %f\n", atof(a[i]), sin(atof(a[i])));	
	dlclose(lib);
	return 0;
}
