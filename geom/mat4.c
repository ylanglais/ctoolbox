#include <stdio.h>
#include <stdlib.h>

#include "mat4.h"

static mat4_t __mat4_null  = {{{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}}};
static mat4_t __mat4_ident = {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}};

mat4_t mat4_null()  {return __mat4_null; }
mat4_t mat4_ident() {return __mat4_ident;} 
	
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
mat4_add(mat4_t A, mat4_t B) {
    register mat4_t C;
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
mat4_sub(mat4_t A, mat4_t B) {
    register mat4_t C;
    C.a[0][0] = A.a[0][0] - B.a[0][0];
    C.a[0][1] = A.a[0][1] - B.a[0][1];
    C.a[0][2] = A.a[0][2] - B.a[0][2];
    C.a[0][3] = A.a[0][3] - B.a[0][3];
    C.a[1][0] = A.a[1][0] - B.a[1][0];
    C.a[1][1] = A.a[1][1] - B.a[1][1];
    C.a[1][2] = A.a[1][2] - B.a[1][2];
    C.a[1][3] = A.a[1][3] - B.a[1][3];
    C.a[2][0] = A.a[2][0] - B.a[2][0];
    C.a[2][1] = A.a[2][1] - B.a[2][1];
    C.a[2][2] = A.a[2][2] - B.a[2][2];
    C.a[2][3] = A.a[2][3] - B.a[2][3];
    C.a[3][0] = A.a[3][0] - B.a[3][0];
    C.a[3][1] = A.a[3][1] - B.a[3][1];
    C.a[3][2] = A.a[3][2] - B.a[3][2];
    C.a[3][3] = A.a[3][3] - B.a[3][3];
    return C;
}    

mat4_t
mat4_mul(mat4_t A, mat4_t B) {
	register mat4_t C;
	C.a[0][0] = A.a[0][0] * B.a[0][0] + A.a[0][1] * B.a[1][0] + A.a[0][2] * B.a[2][0] + A.a[0][3] * B.a[3][0];
	C.a[0][1] = A.a[0][0] * B.a[0][1] + A.a[0][1] * B.a[1][1] + A.a[0][2] * B.a[2][1] + A.a[0][3] * B.a[3][1];
	C.a[0][2] = A.a[0][0] * B.a[0][2] + A.a[0][1] * B.a[1][2] + A.a[0][2] * B.a[2][2] + A.a[0][3] * B.a[3][2];
	C.a[0][3] = A.a[0][0] * B.a[0][3] + A.a[0][1] * B.a[1][3] + A.a[0][2] * B.a[2][3] + A.a[0][3] * B.a[3][3];
	C.a[1][0] = A.a[1][0] * B.a[0][0] + A.a[1][1] * B.a[1][0] + A.a[1][2] * B.a[2][0] + A.a[1][3] * B.a[3][0];
	C.a[1][1] = A.a[1][0] * B.a[0][1] + A.a[1][1] * B.a[1][1] + A.a[1][2] * B.a[2][1] + A.a[1][3] * B.a[3][1];
	C.a[1][2] = A.a[1][0] * B.a[0][2] + A.a[1][1] * B.a[1][2] + A.a[1][2] * B.a[2][2] + A.a[1][3] * B.a[3][2];
	C.a[1][3] = A.a[1][0] * B.a[0][3] + A.a[1][1] * B.a[1][3] + A.a[1][2] * B.a[2][3] + A.a[1][3] * B.a[3][3];
	C.a[2][0] = A.a[2][0] * B.a[0][0] + A.a[2][1] * B.a[1][0] + A.a[2][2] * B.a[2][0] + A.a[2][3] * B.a[3][0];
	C.a[2][1] = A.a[2][0] * B.a[0][1] + A.a[2][1] * B.a[1][1] + A.a[2][2] * B.a[2][1] + A.a[2][3] * B.a[3][1];
	C.a[2][2] = A.a[2][0] * B.a[0][2] + A.a[2][1] * B.a[1][2] + A.a[2][2] * B.a[2][2] + A.a[2][3] * B.a[3][2];
	C.a[2][3] = A.a[2][0] * B.a[0][3] + A.a[2][1] * B.a[1][3] + A.a[2][2] * B.a[2][3] + A.a[2][3] * B.a[3][3];
	C.a[3][0] = A.a[3][0] * B.a[0][0] + A.a[3][1] * B.a[1][0] + A.a[3][2] * B.a[2][0] + A.a[3][3] * B.a[3][0];
	C.a[3][1] = A.a[3][0] * B.a[0][1] + A.a[3][1] * B.a[1][1] + A.a[3][2] * B.a[2][1] + A.a[3][3] * B.a[3][1];
	C.a[3][2] = A.a[3][0] * B.a[0][2] + A.a[3][1] * B.a[1][2] + A.a[3][2] * B.a[2][2] + A.a[3][3] * B.a[3][2];
	C.a[3][3] = A.a[3][0] * B.a[0][3] + A.a[3][1] * B.a[1][3] + A.a[3][2] * B.a[2][3] + A.a[3][3] * B.a[3][3];
	return C;
}

mat4_t
mat4_transpose(mat4_t A) {
	register mat4_t B;
	B.a[0][0] = A.a[0][0]; B.a[0][1] = A.a[1][0]; B.a[0][2] = A.a[2][0]; B.a[0][3] = A.a[3][0]; 
	B.a[1][0] = A.a[0][1]; B.a[1][1] = A.a[1][1]; B.a[1][2] = A.a[2][1]; B.a[1][3] = A.a[3][1]; 
	B.a[2][0] = A.a[0][2]; B.a[2][1] = A.a[1][2]; B.a[2][2] = A.a[2][2]; B.a[2][3] = A.a[3][2];
	B.a[3][0] = A.a[0][3]; B.a[3][1] = A.a[1][3]; B.a[3][2] = A.a[2][3]; B.a[3][3] = A.a[3][3];
	return B;
}
	
double
mat4_det(mat4_t A) {
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


mat4_t
mat4_scale(double sx, double sy, double sz) {
	mat4_t S;
    S = __mat4_ident;
	S.a[0][0] = sx;
	S.a[1][1] = sy;
	S.a[2][2] = sz;
	return S;
}   

mat4_t
mat4_rotation(axis_t axis, double angle) {
	mat4_t R;
	R = __mat4_ident;
	if (axis == _x_) {
		R.a[1][1] =   R.a[2][2] = cos(angle);
		R.a[2][1] = -(R.a[1][2] = sin(angle));
	} else if (axis == _y_) {
		R.a[0][0] =   R.a[2][2] = cos(angle);
		R.a[0][2] = -(R.a[2][0] = sin(angle));
	} else if (axis == _z_) {
		R.a[0][0] =   R.a[1][1] = cos(angle);
		R.a[1][0] = -(R.a[0][1] = sin(angle));
	}
	return R; 
}

mat4_t
mat4_translation(double dx, double dy, double dz) {
	mat4_t T;
	T = __mat4_ident;
	T.a[3][0] = dx; T.a[3][1] = dy; T.a[3][2] = dz;
	return T;
}

mat4_t
mat4_projection(double k, double l, double m) {
	mat4_t T;
	T = __mat4_ident;
	T.a[0][3] = k; T.a[1][3] = l; T.a[2][3] = m;
	return T;
}

vect4_t
vect4_mul_mat4(vect4_t v, mat4_t M) {
	vect4_t V;
	V.a[_x_] = v.a[_x_] * M.a[0][0] + v.a[_y_] * M.a[1][0] + v.a[_z_] * M.a[2][0] + v.a[_t_] * M.a[3][0];
	V.a[_y_] = v.a[_x_] * M.a[0][1] + v.a[_y_] * M.a[1][1] + v.a[_z_] * M.a[2][1] + v.a[_t_] * M.a[3][1];
	V.a[_z_] = v.a[_x_] * M.a[0][2] + v.a[_y_] * M.a[1][2] + v.a[_z_] * M.a[2][2] + v.a[_t_] * M.a[3][2];
	V.a[_t_] = v.a[_x_] * M.a[0][3] + v.a[_y_] * M.a[1][3] + v.a[_z_] * M.a[2][3] + v.a[_t_] * M.a[3][3];
	return V;
}

vect4_t
mat4_solve(mat4_t A, vect4_t B) {
	int i, j;
	double d;
	vect4_t r;
	mat4_t  T;

	d = mat4_det(A);

	for (i = 0; i < 4; i++) {
		T = A;
		for (j = 0; j < 4; j++) {
			T.a[j][i] = B.a[j];
	 	}
		r.a[i] = mat4_det(T) / d;	
	}
	return r;
}

#ifdef _test_mat4_
int
main(void) {
	mat4_t A, B, C;
	vect4_t v, r;
	int i;

	A = mat4_null();
	mat4_dump(A, "A <-- null");
	B = mat4_ident();
	mat4_dump(B, "B <-- ident");
		
	for (i = 0; i < 16; i++) A.a[i / 4][i % 4] = i; 

	mat4_dump(A, "A");
	B = A;
	mat4_dump(B, "B <-- A");
	C = mat4_add(A, B);
	mat4_dump(C, "C <-- A + B");
	C = mat4_sub(A, B);
	mat4_dump(C, "C <-- A - B");
	C = mat4_mul(A, B);
	mat4_dump(C, "C <-- A * B");
	C = mat4_transpose(A);
	mat4_dump(C, "C <-- transpose A");
	
	
	mat4_dump(A, "A");
	printf("mat det(A) = %f\n", mat4_det(A));
	mat4_dump(C, "C");
	printf("mat det(C) = %f\n", mat4_det(C));
	
	printf("mat solve:\n");
	A.a[0][0] =  1;
	A.a[0][1] =  5;
	A.a[0][2] =  3;
	A.a[0][3] =  4;
	A.a[1][0] =  2;
	A.a[1][1] =  3;
	A.a[1][2] =  7;
	A.a[1][3] =  5;
	A.a[2][0] =  3;
	A.a[2][1] =  4;
	A.a[2][2] =  5;
	A.a[2][3] =  6;
	A.a[3][0] =  4;
	A.a[3][1] =  5;
	A.a[3][2] =  6;
	A.a[3][3] =  8;

	//v = {{50, 2, 31}};
	v.a[0] = 5;
	v.a[1] = 6;
	v.a[2] = 7;
	v.a[3] = 8;
	
	r = mat4_solve(A, v);
	mat4_dump(A, "A = ");
	printf("mat4_det(A) = %f\n", mat4_det(A));
	printf("v = {%f, %f, %f, %f}\n", v.a[0], v.a[1], v.a[2], v.a[3]);
	printf("r = {%f, %f, %f, %f}\n", r.a[0], r.a[1], r.a[2], r.a[3]);

	return 0;
}
#endif
