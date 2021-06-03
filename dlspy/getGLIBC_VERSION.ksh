#!/bin/bash
uname | grep -i linux 2>/dev/null 1>&2 || {
	echo "not a linux platform ==> might not work :("
}
cat > dltest.c <<EOF
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
EOF
gcc -o dltest dltest.c -ldl && {
	GLIBC_VERSION=$(nm dltest | grep dlsym | cut -d@ -f2)
	echo "$green$bold$GLIBC_VERSION$norm"
	echo "#define DLSPY_GLIBC_VERSION \"$GLIBC_VERSION\""  > config.h
} || {
	echo "$red${bold}cannot find standard dlsym glibc version string$norm"
	exit 1
}
