#include <stdlib.h>
#include <string.h>

#include "cmbs.h"

/*
 * Thanks to http://www.dcode.fr/combinaisons for the recursive algo and the cardinality
 */

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
		30/06/2016	1.0	initial release

*/

char cmbs_MODULE[]  = "Generate combinasions";
char cmbs_PURPOSE[] = "Generate combinasions";
char cmbs_VERSION[] = "1.0";
char cmbs_DATEVER[] = "30/06/2016";

#if 0
static int _cmbs_ia_push(int *a, int i) {
	int *p;

	for (p = a; *p != -1; p++);
	*p = i;
	return i;
}
#endif

#if 0
static unsigned long long
_cmbs_fact(int n) {
	unsigned long long f;
	
	if (n < 1) return 1;
	f = n;
	while (--n > 0) f *= n;
	return f;
}
#endif 
unsigned long long
cmbs_card(int n, int k) {
	if (k == n) return 1;
	if (k == 1) return n;

	unsigned long long r = 1;
    for (unsigned long long d = 1; d <= k; ++d) {
        r *= n--;
        r /= d;
    }
    return r;
}

static unsigned long long 
_cmbs_choose(int n, int k) {
	if (n < 0 || k < 0 || n < k) return 0;
	if (n == k) return 1;

	unsigned long long imax, delta;

	if (k < n - k) { 
		delta = n - k;
		imax  = k;
	} else {
		delta = k;
		imax  = n - k;
	}

	unsigned long long ans = delta + 1;
	unsigned long long i;
	
	for (i = 2; i <= imax; i++) {
		ans = (ans *(delta + i)) / i;
	}
	return ans;
}

static unsigned long long
_cmbs_largest(long a, long b, long x) {
	long v = a - 1;
	while (_cmbs_choose(v, b) > x) --v;
	return v;
}


int *
cmbs_i(int n, int k, int i) {
	int *result;
	int j;
	
	if (!(result = (int *) malloc((k + 1) * sizeof(int)))) {
		return NULL;
	}
	memset(result, -1, (k + 1) * sizeof(int));

	long a = n;
	long b = k;
	long x = _cmbs_choose(n, k) - 1 - i;    // x is the "dual" of i
 
	for (j = 0; j < k; j++, b--) {
		result[j] = _cmbs_largest(a, b, x); // largest value v, where v < a and vCb < x    
		x -= _cmbs_choose(result[j], b);
		a = result[j];
	}

	for (j = 0; j < k; j++) {
		result[j] = (n - 1) - result[j];
	}

	return result;
}

#ifdef _test_cmbs_
#include <stdio.h>
static void cmbs_iaprint(int *a) {
	int i;
	if (!a) return;
	for (i = 0; a[i] >= 0; i++) {
		printf("%d ", a[i]);
	}
}
int main(int n, char *a[]) {
	int i, k, m, cardk;

	printf("n = %d\n", n);

	if (n < 3) {
		printf("%s usage: %s n k\n", a[0], a[0]);
		return 1;
	}
	
	m = atoi(a[1]);
	k = atoi(a[2]);

	printf("m = %d\n", m);
	printf("k = %d\n", k);

	cardk = cmbs_card(m, k);

	printf("cmbs_card(%d, %d) = %d\n", m, k, cardk);
	
	for (i = 0; i < cardk; i++) {
		int *r;
		
		r = cmbs_i(m, k, i);
		printf("%5d: ", i);
		cmbs_iaprint(r);
		printf("\n");
		if (r) free(r);
	} 
	return 0;
}
#endif
