#include <stdlib.h>

#include <tbx/futl.h>

#include "nq.h"


int main(int n, char *a[]) {
	pnq_t  q;
	if (!(q = nq_load())) return 1;
	nq_dump(q);
	return 0;
}
