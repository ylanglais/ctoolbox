#include <stdio.h>
#include <stdlib.h>

#define LOOP 100000000


#ifdef matvect4HIGHPRECISION 
typedef double real;
#include <math.h>
#else 
typedef float  real;
/*#include "trigo.c"*/
#endif

#define matSIZE 4

typedef struct {
	real a[4][4]; 
} mat4_t, *pmat4_t;

typedef enum {
	_x_ = 0,
	_y_ = 1,
	_z_ = 2
} coord_t;

static mat4_t __mat4_null  = {{{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}}};
static mat4_t __mat4_ident = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}};

mat4_t mat4_null()  {return __mat4_null; }
mat4_t mat4_ident() {return __mat4_ident;} 
	
#define mat4_dump(A, str) mat4_fdump(NULL, (A), (str))

void
mat4_fdump(FILE *f, mat4_t A, char *name) {
	int i, j;
	if (f == NULL) f = stdout;
	if (name) fprintf(f, "%s:\n",  name);
	for (i = 0; i < matSIZE; i++) {
		fprintf(f, " | ");
		for (j = 0; j < matSIZE; j++) fprintf(f, "%6.2f ", A.a[i][j]);			
		fprintf(f, "\n");
	}
}

mat4_t
mat4_add_loop(mat4_t A, mat4_t B) {
	int i, j;
	mat4_t C;
	for (i = 0; i < matSIZE; i++) for (j = 0; j < matSIZE; j++) 
	C.a[i][j] = A.a[i][j] + B.a[i][j];
	return C;
} 

mat4_t
mat4_add_expl(mat4_t A, mat4_t B) {
    mat4_t C;
    C.a[0][0] = A.a[0][0] + B.a[0][0];
    C.a[0][1] = A.a[0][1] + B.a[0][1];
    C.a[0][2] = A.a[0][2] + B.a[0][2];
    C.a[0][3] = A.a[0][3] + B.a[0][3];
    C.a[1][0] = A.a[1][0] + B.a[1][0];
    C.a[1][1] = A.a[1][1] + B.a[1][1];
    C.a[1][2] = A.a[1][2] + B.a[1][2];
    C.a[1][3] = A.a[1][3] + B.a[1][3];
    C.a[2][0] = A.a[2][0] + B.a[2][0];
    C.a[2][1] = A.a[2][1] + B.a[2][1];
    C.a[2][2] = A.a[2][2] + B.a[2][2];
    C.a[2][3] = A.a[2][3] + B.a[2][3];
    C.a[3][0] = A.a[3][0] + B.a[3][0];
    C.a[3][1] = A.a[3][1] + B.a[3][1];
    C.a[3][2] = A.a[3][2] + B.a[3][2];
    C.a[3][3] = A.a[3][3] + B.a[3][3];
    return C;
}    

mat4_t
mat4_mul_loop(mat4_t A, mat4_t B) {
	register int i, j, k;
	mat4_t C;
	
	C = __mat4_null;
	for (i = 0; i < matSIZE; i++) 
		for (j = 0; j < matSIZE; j++) 
			for (k = 0; k < matSIZE; k++) 
				C.a[i][j] += A.a[i][k] * B.a[k][j];
	return C;
}

mat4_t
mat4_mul_expl(mat4_t A, mat4_t B) {
	mat4_t C;
	C.a[0][0] = A.a[0][0] * B.a[0][0] +A.a[0][1] * B.a[1][0] +A.a[0][2] * B.a[2][0] +A.a[0][3] * B.a[3][0];
	C.a[0][1] = A.a[0][0] * B.a[0][1] +A.a[0][1] * B.a[1][1] +A.a[0][2] * B.a[2][1] +A.a[0][3] * B.a[3][1];
	C.a[0][2] = A.a[0][0] * B.a[0][2] +A.a[0][1] * B.a[1][2] +A.a[0][2] * B.a[2][2] +A.a[0][3] * B.a[3][2];
	C.a[0][3] = A.a[0][0] * B.a[0][3] +A.a[0][1] * B.a[1][3] +A.a[0][2] * B.a[2][3] +A.a[0][3] * B.a[3][3];
	C.a[1][0] = A.a[1][0] * B.a[0][0] +A.a[1][1] * B.a[1][0] +A.a[1][2] * B.a[2][0] +A.a[1][3] * B.a[3][0];
	C.a[1][1] = A.a[1][0] * B.a[0][1] +A.a[1][1] * B.a[1][1] +A.a[1][2] * B.a[2][1] +A.a[1][3] * B.a[3][1];
	C.a[1][2] = A.a[1][0] * B.a[0][2] +A.a[1][1] * B.a[1][2] +A.a[1][2] * B.a[2][2] +A.a[1][3] * B.a[3][2];
	C.a[1][3] = A.a[1][0] * B.a[0][3] +A.a[1][1] * B.a[1][3] +A.a[1][2] * B.a[2][3] +A.a[1][3] * B.a[3][3];
	C.a[2][0] = A.a[2][0] * B.a[0][0] +A.a[2][1] * B.a[1][0] +A.a[2][2] * B.a[2][0] +A.a[2][3] * B.a[3][0];
	C.a[2][1] = A.a[2][0] * B.a[0][1] +A.a[2][1] * B.a[1][1] +A.a[2][2] * B.a[2][1] +A.a[2][3] * B.a[3][1];
	C.a[2][2] = A.a[2][0] * B.a[0][2] +A.a[2][1] * B.a[1][2] +A.a[2][2] * B.a[2][2] +A.a[2][3] * B.a[3][2];
	C.a[2][3] = A.a[2][0] * B.a[0][3] +A.a[2][1] * B.a[1][3] +A.a[2][2] * B.a[2][3] +A.a[2][3] * B.a[3][3];
	C.a[3][0] = A.a[3][0] * B.a[0][0] +A.a[3][1] * B.a[1][0] +A.a[3][2] * B.a[2][0] +A.a[3][3] * B.a[3][0];
	C.a[3][1] = A.a[3][0] * B.a[0][1] +A.a[3][1] * B.a[1][1] +A.a[3][2] * B.a[2][1] +A.a[3][3] * B.a[3][1];
	C.a[3][2] = A.a[3][0] * B.a[0][2] +A.a[3][1] * B.a[1][2] +A.a[3][2] * B.a[2][2] +A.a[3][3] * B.a[3][2];
	C.a[3][3] = A.a[3][0] * B.a[0][3] +A.a[3][1] * B.a[1][3] +A.a[3][2] * B.a[2][3] +A.a[3][3] * B.a[3][3];
	return C;
}

mat4_t
mat4_mul_hybr(mat4_t A, mat4_t B) {
	register int i, j;
	mat4_t C;
	for(i = 0; i < 4; i++) for(j = 0; j < 4; j++)
		C.a[i][j] = 
			A.a[i][0] * B.a[0][j] + 
			A.a[i][1] * B.a[1][j] + 
			A.a[i][2] * B.a[2][j] + 
			A.a[i][3] * B.a[3][j];
	return C;
}

typedef real MATRIX4[16];
typedef real MATRIX3[9];

void 
mat4_submat(MATRIX4 mr, MATRIX3 mb, int i, int j) {
	int ti, tj, idst, jdst;

	for (ti = 0; ti < 4; ti++) {
        if      (ti < i) idst = ti; 
		else if (ti > i) idst = ti-1;

        for (tj = 0; tj < 4; tj++) {
			if      (tj < j) jdst = tj;
			else if (tj > j) jdst = tj - 1;

			if (ti != i && tj != j) mb[idst * 3 + jdst] = mr[ti * 4 + tj];
		}
	}
}

real
mat3_det(MATRIX3 mat) {
	float det;
	det = mat[0] * (mat[4] * mat[8] - mat[7] * mat[5])
		- mat[1] * (mat[3] * mat[8] - mat[6] * mat[5])
		+ mat[2] * (mat[3] * mat[7] - mat[6] * mat[4]);
	
	/* printf("m3 I  = %f\n", det); */
	return det;
}

real 
mat4_det_loop(MATRIX4 mr) {
	real det, result = 0, i = 1;
	MATRIX3 msub3;
	int     n;

	for (n = 0; n < 4; n++, i *= -1) {
        mat4_submat(mr, msub3, 0, n);
        det     = mat3_det(msub3);
        result += mr[n] * det * i;
		/* printf("m4 I  = %f\n", result); */
	}
	return result;
}

real
mat4_det_expl(mat4_t A) {
	return A.a[0][0] * (  A.a[1][1] * (A.a[2][2] * A.a[3][3] - A.a[3][2] * A.a[2][3]) 
		 			    - A.a[1][2] * (A.a[2][1] * A.a[3][3] - A.a[3][1] * A.a[2][3])
					    + A.a[1][3] * (A.a[2][1] * A.a[3][2] - A.a[3][1] * A.a[2][2]))
		 - A.a[0][1] * (  A.a[1][0] * (A.a[2][2] * A.a[3][3] - A.a[3][2] * A.a[2][3])
					    - A.a[1][2] * (A.a[2][0] * A.a[3][3] - A.a[3][0] * A.a[2][3])
					    + A.a[1][3] * (A.a[2][0] * A.a[3][2] - A.a[3][0] * A.a[2][2]))
		 + A.a[0][2] * (  A.a[1][0] * (A.a[2][1] * A.a[3][3] - A.a[3][1] * A.a[2][3])
					    - A.a[1][1] * (A.a[2][0] * A.a[3][3] - A.a[3][0] * A.a[2][3])
					    + A.a[1][3] * (A.a[2][0] * A.a[3][1] - A.a[3][0] * A.a[2][1]))
	     - A.a[0][3] * (  A.a[1][0] * (A.a[2][1] * A.a[3][2] - A.a[3][1] * A.a[2][2])
					    - A.a[1][1] * (A.a[2][0] * A.a[3][2] - A.a[3][0] * A.a[2][2])
					    + A.a[1][2] * (A.a[2][0] * A.a[3][1] - A.a[3][0] * A.a[2][1]));
}
	
#include <tbx/coverage.h>
#define tstruct struct timeval

int main(void) {
	mat4_t A, B, C;
	int i;
	tstruct t0, t1;
	char *p;
	MATRIX4 M;

	A = mat4_null();
	mat4_dump(A, "A <-- null");
	B = mat4_ident();
	mat4_dump(B, "B <-- ident");
		
	for (i = 0; i < 16; i++) A.a[i / 4][i % 4] = i; 

	mat4_dump(A, "A");
	B = A;
	mat4_dump(B, "B <-- A");
	C = mat4_add_loop(A, B);
	mat4_dump(C, "C <-- A + B (loop method)");
	C = mat4_add_expl(A, B);
	mat4_dump(C, "C <-- A + B (explicit method)");
	C = mat4_mul_loop(A, B);
	mat4_dump(C, "C <-- A * B (loop method)");
	C = mat4_mul_expl(A, B);
	mat4_dump(C, "C <-- A * B (explicit method)");
	C = mat4_mul_hybr(A, B);
	mat4_dump(C, "C <-- A * B (hybrid method)");


	printf("Executing %d mat add w/ loop method...     ", LOOP);
	t0 = cov_time_get();
	for (i = 0; i < LOOP; i++) {
		mat4_add_loop(A, B);
	}
	t1 = cov_time_sub(cov_time_get(), t0);
	printf("in %9s\n", p = cov_time_fmt(t1));
	free(p);

	printf("Executing %d mat add w/ explicit method... ", LOOP);
	t0 = cov_time_get();
	for (i = 0; i < LOOP; i++) {
		mat4_add_expl(A, B);
	}
	t1 = cov_time_sub(cov_time_get(), t0);
	printf("in %9s\n", p = cov_time_fmt(t1));
	free(p);

    printf("Executing %d mat mul w/ loop method...     ", LOOP);
    t0 = cov_time_get();
    for (i = 0; i < LOOP; i++) {
        mat4_mul_loop(A, B);
    }
    t1 = cov_time_sub(cov_time_get(), t0);
    printf("in %9s\n", p = cov_time_fmt(t1));
    free(p);
    
    printf("Executing %d mat mul w/ explicit method... ", LOOP);
    t0 = cov_time_get();
    for (i = 0; i < LOOP; i++) {
        mat4_mul_expl(A, B);
    }
    t1 = cov_time_sub(cov_time_get(), t0);
    printf("in %9s\n", p = cov_time_fmt(t1));
    
    printf("Executing %d mat mul w/ hybrid method...   ", LOOP);
    t0 = cov_time_get();
    for (i = 0; i < LOOP; i++) {
        mat4_mul_hybr(A, B);
    }
    t1 = cov_time_sub(cov_time_get(), t0);
    printf("in %9s\n", p = cov_time_fmt(t1));
    free(p);

	for (i = 0; i < 16; i++) M[i] = A.a[i / 4][i % 4] = i; 
	printf("Executing %d mat_det_loop... ", LOOP);
	t0 = cov_time_get();
	for (i = 0; i < LOOP; i++) mat4_det_loop(M);
	t1 = cov_time_sub(cov_time_get(), t0);
	printf("%9s\n", p = cov_time_fmt(t1));
	free(p);
	printf("Executing %d mat_det_expl...", LOOP);
	t0 = cov_time_get();
	for (i = 0; i < LOOP; i++) mat4_det_expl(A);
	t1 = cov_time_sub(cov_time_get(), t0);
	printf("%9s\n", p = cov_time_fmt(t1));
	free(p);
	return 0;
}
	
