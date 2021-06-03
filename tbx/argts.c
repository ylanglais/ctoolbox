#include <stdlib.h>
#include <string.h>

#include "argts.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
		30/05/2016	1.1	Creation

*/

char argts_MODULE[]  = "Generate arrangements";
char argts_PURPOSE[] = "Generate arrangements";
char argts_VERSION[] = "1.1";
char argts_DATEVER[] = "30/05/2016";

static int argts_iapush(int *a, int i) {
	int *p;

	for (p = a; *p != -1; p++);
	*p = i;
	return i;
}
static int argts_iaremove(int *a, int i) {
	int v;
	int *p;
	v = a[i];
	for (p = a + i; *p != -1; p++) *p = p[1];
	return v;
}
unsigned long long
argts_fact(int n) {
	unsigned long long f;
	
	if (n < 1) return 1;
	f = n;
	while (--n > 0) f *= n;
	return f;
}
unsigned long long
argts_card(int n, int k) {
	int i;
	unsigned long long f;
	if (n < 1) return 1;

	f = 1;
	for (i = n; i > (n - k); i--) {
		f *= i;
	}
	return f;
}
int *
argts_i(int n, int k, int i) {
	int *avail, *result;
	int j;
	
	if (!(result = (int *) malloc((k + 1) * sizeof(int)))) {
		return NULL;
	}
	if (!(avail  = (int *) malloc((n + 1) * sizeof(int)))) {
		free(result);
		return NULL;
	}
	memset(result, -1, (k + 1) * sizeof(int));
	memset(avail,  -1, (n + 1) * sizeof(int));

	for (j = 0; j < n; j++) {
		avail[j] = j;
	}
	
	while (k > 0) {
		int card, d;
		card = argts_card(n, k) / n;
		d = i / card;
		argts_iapush(result, argts_iaremove(avail, d));
		k--; n--; i %= card;
	}
	free(avail);
	return result;
}

#ifdef _test_argts_
#include <stdio.h>
static void argts_iaprint(int *a) {
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

	cardk = argts_card(m, k);

	printf("argts_card(%d, %d) = %d\n", m, k, cardk);
	
	for (i = 0; i < cardk; i++) {
		int *r;
		
		r = argts_i(m, k, i);
		printf("%5d: ", i);
		argts_iaprint(r);
		printf("\n");
		if (r) free(r);
	} 
	return 0;
}
#endif
