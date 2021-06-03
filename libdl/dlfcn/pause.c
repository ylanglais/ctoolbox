#include <unistd.h>
#include <dlfcn.h>

int main(void) {
	void *p;
	p = dlopen("lib1.so", RTLD_LAZY);
	pause();
	return 0;
}
