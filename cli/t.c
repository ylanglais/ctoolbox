
#include <stdlib.h>
#include <dlfcn.h>

#include <stdio.h>

#include <tbx/tstmr.h>

typedef tstamp_t (*tsparse_f)(char *);
typedef int (*main_f)(int, char *[], char *[]);

int main(int n, char *a[], char *env[]) {
	void *dl;
	tsparse_f parser;
	main_f dlmain;
	char *dla[] = { "dlmain", "3" };

	if (!(dl = dlopen("countdown", RTLD_LAZY))) {
		printf("cannot load countdown: %s\n", dlerror());
		exit(1);
	}
	if (!(parser = (tsparse_f) dlsym(dl, "ts_parse"))) {
		printf("cannot dlsym ts_parse: %s\n", dlerror());
		dlclose(dl);
		exit(2);
	}

	tstamp_t ts = parser("30"); 
	printf("ts = %u.%u\n", ts.tv_sec, ts.tv_usec);

	//if (!(dlmain = (main_f) dlsym(dl, "main"))) {
	if (!(dlmain = (int (*)(int, char *[], char *[])) dlsym(dl, "main"))) {
		printf("cannot dlsym main: %s\n", dlerror());
		dlclose(dl);
		exit(3);
	}
	printf("call dlmain(2, {\"dlmain\", \"3\"}, env):\n");
	int i = dlmain(2, dla, env);
	printf("dlmain returned %d\n", i);

	dlclose(dl);
	exit(0);
}

