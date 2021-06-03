#include <stdlib.h>
#include <stdio.h>

size_t
mprof_closest_upper_power(size_t val) {
	size_t n;
	for (n = 1; n < val ; n <<= 1) printf("\t%d\n", n);
	return n;
}

int
mprof_power_of_2(size_t val) {
	int i, n;
	for (i = 1; val > (1 << i); i++);
	return i;
}

void
mprof_upper_closest_pow_of_2(size_t val, int *pwr, size_t *closest) {
	int i, n;
	for (i = 1, n = 1; n < val; n = 1 << ++i);
	*pwr   = i;
	*closest = n;
}
 
int 
main(int n, char *a[]) {
	int i, v, p;
	size_t j;

	i = 1;
	while (i < n) {
		v = atol(a[i++]);
		p = mprof_closest_upper_power(v);
		printf("% 6d -> % 6d = 2^%d\n", v, p, mprof_power_of_2(p));
		mprof_upper_closest_pow_of_2(v, &p, &j);
		printf("       -> % 6d = 2^%d\n", j, p);
	}

	return 0;
}
