#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
int main(int n, char *a[]) {
    int i;
    void *lib;
    double (*math)(double);

    if (!(lib  = dlopen("/usr/lib/libm-2.29.so", RTLD_LAZY))) {
		printf("ERROR: %s\n", dlerror());
		return 1;
	}
    if (!(math = (double (*)(double)) dlsym(lib, a[0]))) return 2;

    for (i = 1; i < n; i++) printf("%s(%f) = %f\n", a[0], atof(a[i]), math(atof(a[i])));
    dlclose(lib);
    return 0;
}
