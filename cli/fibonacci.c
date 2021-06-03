#include <stdlib.h>
#include <stdio.h>

int main(int n, char *v[]) {
	int i, max = 10;	
	unsigned long long t, a, b;

	if (n > 1) max = atoi(v[1]);
	a = 0;
	b = 1;

	for (i = 0; i < max; i++) {
		t = a;
		a = b;
		b = t + b;
		printf("u[%4d] = %33llu\n", i + 1, b);
	}
	
	return 0;
}
