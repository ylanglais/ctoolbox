#include <stdlib.h>
#include <dlfcn.h>
int main(void) {
	char *t;
	if (!dlsym(RTLD_DEFAULT, "titi")) {
		t = dlerror();
		printf("%x : err = %s\n", t, t);
	}
	if (!dlsym(RTLD_DEFAULT, "toto")) {
		t = dlerror();
		printf("%x : err = %s\n", t, t);
	}
	
	return 0;
}
