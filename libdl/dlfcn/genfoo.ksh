for i in 1 2 3 
do
cat >gt; lib$i.c <lt;<lt;EOF
#include <lt;stdio.h>gt;
#include <lt;dlfcn.h>gt;
void foo() {
	void (*next_foo)();
	printf("lib$i.foo()\n");	
	next_foo = (void (*)()) dlsym(RTLD_NEXT, "foo");
	next_foo();
}
EOF
gcc -G lib$i.c -o lib$i.so
done
cat >gt; lib4.c <lt;<lt;EOF
#include <lt;stdio.h>gt;
#include <lt;dlfcn.h>gt;
void foo() {
	printf("lib4.foo()\n");	
}
EOF
gcc -G lib4.c -o lib4.so
cat >gt; foo.c <lt;<lt;EOF
#include <lt;dlfcn.h>gt; 
extern void foo();
int main() {
	foo();
	return 0;
}
EOF
gcc foo.c -o foo -L. -l1 -l2 -l3 -l4 -ldl 
