#include <stdlib.h>
#include <stdio.h>

#include "mat3.h"

static mat3_t __mat3_null  = {{{0,0,0}, {0,0,0}, {0,0,0}}};
static mat3_t __mat3_ident = {{{1,0,0}, {0,1,0}, {0,0,1}}};

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
    C.a[0][0] = A.a[0][0] + B.a[0][0];
    C.a[0][1] = A.a[0][1] + B.a[0][1];
    C.a[0][2] = A.a[0][2] + B.a[0][2];

    C.a[1][0] = A.a[1][0] + B.a[1][0];
    C.a[1][1] = A.a[1][1] + B.a[1][1];
    C.a[1][2] = A.a[1][2] + B.a[1][2];

    C.a[2][0] = A.a[2][0] + B.a[2][0];
    C.a[2][1] = A.a[2][1] + B.a[2][1];
    C.a[2][2] = A.a[2][2] + B.a[2][2];
    return C;
}    

mat3_t
mat3_sub(mat3_t A, mat3_t B) {
    register mat3_t C;
    C.a[0][0] = A.a[0][0] - B.a[0][0];
    C.a[0][1] = A.a[0][1] - B.a[0][1];
    C.a[0][2] = A.a[0][2] - B.a[0][2];

    C.a[1][0] = A.a[1][0] - B.a[1][0];
    C.a[1][1] = A.a[1][1] - B.a[1][1];
    C.a[1][2] = A.a[1][2] - B.a[1][2];

    C.a[2][0] = A.a[2][0] - B.a[2][0];
    C.a[2][1] = A.a[2][1] - B.a[2][1];
    C.a[2][2] = A.a[2][2] - B.a[2][2];
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
	
double
mat3_det(mat3_t A) {
	return 
		  A.a[0][0] * (A.a[1][1] * A.a[2][2] - A.a[2][1] * A.a[1][2]) -
		  A.a[0][1] * (A.a[1][0] * A.a[2][2] - A.a[2][0] * A.a[1][2]) +
		  A.a[0][2] * (A.a[1][0] * A.a[2][1] - A.a[2][0] * A.a[1][1] );
}

mat3_t
mat3_scale(double sx, double sy) {
	mat3_t S;
    S = __mat3_ident;
	S.a[0][0] = sx;
	S.a[1][1] = sy;
	return S;
}   

mat3_t
mat3_rotation(double angle) {
	mat3_t R;
	R = __mat3_ident;
	R.a[0][0] =   R.a[1][1] = cos(angle);
	R.a[1][0] = -(R.a[0][1] = sin(angle));
	return R; 
}

mat3_t
mat3_translation(double dx, double dy) {
	mat3_t T;
	T = __mat3_ident;
	T.a[2][0] = dx; T.a[2][1] = dy; 
	return T;
}

vect3_t
mat3_mul_vect3(mat3_t M, vect3_t v) {
	vect3_t V;
	V.a[_x_] = v.a[_x_] * M.a[0][0] + v.a[_y_] * M.a[1][0] + v.a[_z_] * M.a[2][0];
	V.a[_y_] = v.a[_x_] * M.a[0][1] + v.a[_y_] * M.a[1][1] + v.a[_z_] * M.a[2][1];
	V.a[_z_] = v.a[_x_] * M.a[0][2] + v.a[_y_] * M.a[1][2] + v.a[_z_] * M.a[2][2];
	return V;
}

vect3_t
mat3_solve(mat3_t A, vect3_t B) {
	int i, j; 
	double d;
	vect3_t r;
	mat3_t  T;

	d = mat3_det(A);

	for (i = 0; i < 3; i++) {
		T = A;
		for (j = 0; j < 3; j++) {
			T.a[j][i] = B.a[j];
	 	}
		r.a[i] = mat3_det(T) / d;	
	}
	return r;
}

#ifdef _test_mat3_
int
main(void) {
	mat3_t A, B, C;
	vect3_t v, r;
	int i;

	A = mat3_null();
	mat3_dump(A, "A <-- null");
	B = mat3_ident();
	mat3_dump(B, "B <-- ident");
		
	for (i = 0; i < matSIZE * matSIZE; i++) A.a[i / matSIZE][i % matSIZE] = i; 

	mat3_dump(A, "A");
	B = A;
	mat3_dump(B, "B <-- A");
	C = mat3_add(A, B);
	mat3_dump(C, "C <-- A + B");
	C = mat3_sub(A, B);
	mat3_dump(C, "C <-- A - B");
	C = mat3_mul(A, B);
	mat3_dump(C, "C <-- A * B");
	C = mat3_transpose(A);
	mat3_dump(C, "C <-- transpose A");
	
	mat3_dump(A, "A");
	printf("mat det(A) = %f\n", mat3_det(A));
	mat3_dump(C, "C");
	printf("mat det(C) = %f\n", mat3_det(C));
		
	printf("mat solve:\n");
	//A = {{{1, 3, 4}, {3, 5, -4}, {4, 7, -2}}};
	A.a[0][0] =  1;
	A.a[0][1] =  3;
	A.a[0][2] =  4;
	A.a[1][0] =  3;
	A.a[1][1] =  5;
	A.a[1][2] = -4;
	A.a[2][0] =  4;
	A.a[2][1] =  7;
	A.a[2][2] = -2;

	//v = {{50, 2, 31}};
	v.a[0] = 50;
	v.a[1] = 2;
	v.a[2] = 31;
	
	r = mat3_solve(A, v);
	mat3_dump(A, "A = ");
	printf("mat3_det(A) = %f\n", mat3_det(A));
	printf("v = {%f, %f, %f}\n", v.a[0], v.a[1], v.a[2]);
	printf("r = {%f, %f, %f}\n", r.a[0], r.a[1], r.a[2]);
	
	
	


	return 0;		
}
	
#endif
