#include <stdlib.h>
#include <string.h>

#include "tuples.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	28/06/2016	1.0  creation
	04/08/2016  1.1  add changelog 
*/   

char tuples_MODULE[]  = "Generate tuples";
char tuples_PURPOSE[] = "Generate tuples";
char tuples_VERSION[] = "1.1";
char tuples_DATEVER[] = "04/08/2016";

static int tuples_iapush(int *a, int i) {
	int *p;

	for (p = a; *p != -1; p++);
	*p = i;
	return i;
}

unsigned long long
tuples_card(int n, int k) {
	int i;
	unsigned long long f;

	for (f = 1, i = 0; i < k; i++)
		f *= n; 

	return f;
}
int *
tuples_i(int n, int k, unsigned long long i) {
	int *result;
	unsigned long long d, card;
	
	card = tuples_card(n, k);

	if (!(result = malloc((k + 1) * sizeof(int)))) return NULL; 
	memset(result, -1, (k + 1) * sizeof(int));

	if (i > card) return NULL;

	while (k > 0) {
		d = i % n;
		tuples_iapush(result, d);
		k--; 
		i /= n;
	}
	return result;
}

#ifdef _test_tuples_
#include <stdio.h>
static void tuples_iaprint(int *a) {
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

	cardk = tuples_card(m, k);

	printf("tuples_card(%d, %d) = %d\n", m, k, cardk);
	
	for (i = 0; i < cardk; i++) {
		int *r;
		
		r = tuples_i(m, k, i);
		printf("%5d: ", i);
		tuples_iaprint(r);
		printf("\n");
		if (r) free(r);
	} 
	return 0;
}
#endif
