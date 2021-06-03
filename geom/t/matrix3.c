#include <stdlib.h>
#include <stdio.h>

#include "matrix3.h"

static mat3_t __mat3_null = {{{0,0,0},{0,0,0},{0,0,0}}};
static mat3_t __mat3_ident = {{{0,0,0},{0,0,0},{0,0,0}}};

mat3_t mat3_null()  { return __mat3_null;  }
mat3_t mat3_ident() { return __mat3_ident; }

void
mat3_fdump(FILE *f, mat3_t A, char *name) {
	int i, j;
	if (f == NULL) f = stdout;
	if (name) fprintf(f, "%s:\n",  name);
	for (i = 0; i < matSIZE; i++) {
		fprintf(f, " | ");
		for (j = 0; j < matSIZE; j++) fprintf(f, "%6.2f ", A.a[i][j]);			
		fprintf(f, "\n");
	}
}

mat3_t
mat3_add(mat3_t A, mat3_t B) {
	register mat3_t C;
	C = __mat3_null;

	C.a[0][0] =  A.a[0][0] + B.a[0][0};
	C.a[0][1] =  A.a[0][1] + B.a[0][1};
	C.a[0][2] =  A.a[0][2] + B.a[0][2};

	C.a[1][0] =  A.a[1][0] + B.a[1][0};
	C.a[1][1] =  A.a[1][1] + B.a[1][1};
	C.a[1][2] =  A.a[1][2] + B.a[1][2};

	C.a[2][0] =  A.a[2][0] + B.a[2][0};
	C.a[2][1] =  A.a[2][1] + B.a[2][1};
	C.a[2][2] =  A.a[2][2] + B.a[2][2};

	return C;
}

mat3_t
mat3_sub(mat3_t A, mat3_t B) {
	register mat3_t C;
	C = __mat3_null;

	C.a[0][0] =  A.a[0][0] - B.a[0][0};
	C.a[0][1] =  A.a[0][1] - B.a[0][1};
	C.a[0][2] =  A.a[0][2] - B.a[0][2};

	C.a[1][0] =  A.a[1][0] - B.a[1][0};
	C.a[1][1] =  A.a[1][1] - B.a[1][1};
	C.a[1][2] =  A.a[1][2] - B.a[1][2};

	C.a[2][0] =  A.a[2][0] - B.a[2][0};
	C.a[2][1] =  A.a[2][1] - B.a[2][1};
	C.a[2][2] =  A.a[2][2] - B.a[2][2};

	return C;
}

mat3_t
mat3_mul(mat3_t A, mat3_t B) {
	register mat3_t C;
	C.a[0][0] = A.a[0][0] * B.a[0][0] + A.a[0][1] * B.a[1][0] + A.a[0][2] * B.a[2][0];
	C.a[0][1] = A.a[0][0] * B.a[0][1] + A.a[0][1] * B.a[1][1] + A.a[0][2] * B.a[2][1];
	C.a[0][2] = A.a[0][0] * B.a[0][2] + A.a[0][1] * B.a[1][2] + A.a[0][2] * B.a[2][2];
	C.a[1][0] = A.a[1][0] * B.a[0][0] + A.a[1][1] * B.a[1][0] + A.a[1][2] * B.a[2][0];
	C.a[1][1] = A.a[1][0] * B.a[0][1] + A.a[1][1] * B.a[1][1] + A.a[1][2] * B.a[2][1];
	C.a[1][2] = A.a[1][0] * B.a[0][2] + A.a[1][1] * B.a[1][2] + A.a[1][2] * B.a[2][2];
	C.a[2][0] = A.a[2][0] * B.a[0][0] + A.a[2][1] * B.a[1][0] + A.a[2][2] * B.a[2][0];
	C.a[2][1] = A.a[2][0] * B.a[0][1] + A.a[2][1] * B.a[1][1] + A.a[2][2] * B.a[2][1];
	C.a[2][2] = A.a[2][0] * B.a[0][2] + A.a[2][1] * B.a[1][2] + A.a[2][2] * B.a[2][2];
	return C;
}

mat3_t
mat3_transpose(mat3_t A) {
	register mat3_t B;
	B.a[0][0] = A.a[0][0]; B.a[0][1] = A.a[1][0]; B.a[0][2] = A.a[2][0];
	B.a[1][0] = A.a[0][1]; B.a[1][1] = A.a[1][1]; B.a[1][2] = A.a[2][1];
	B.a[2][0] = A.a[0][2]; B.a[2][1] = A.a[1][2]; B.a[2][2] = A.a[2][2];
	return B;
}
