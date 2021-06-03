#ifndef _matrix4_h_
#define _matrix4_h_

#include <stdio.h>
#include <math.h>
#define trig_sin(x)  sin(x)
#define trig_cos(x)  cos(x)

#define matSIZE 4

typedef struct {
	double a[matSIZE];
} vect4_t, *pvect4_t;

typedef struct {
	double a[matSIZE][matSIZE]; 
} mat4_t, *pmat4_t;

typedef enum {
	_x_ = 0,
	_y_ = 1,
	_z_ = 2,
	_t_ = 3
} axis_t;

mat4_t mat4_null();
mat4_t mat4_ident();
#define mat4_dump(A, str) mat4_fdump(NULL, (A), (str))
void     mat4_fdump(FILE *f, mat4_t A, char *name);
mat4_t   mat4_add(mat4_t A, mat4_t B);
mat4_t   mat4_sub(mat4_t A, mat4_t B);
mat4_t   mat4_mul(mat4_t A, mat4_t B);
mat4_t   mat4_transpose(mat4_t A);
double   mat4_det(mat4_t A);
vect4_t	 mat4_solve(mat4_t A, vect4_t v);
mat4_t   mat4_scale(double sx, double sy, double sz);
mat4_t   mat4_rotation(axis_t axis, double angle);
mat4_t   mat4_translation(double dx, double dy, double dz);
mat4_t   mat4_projection(double k, double l, double m);
vect4_t  vect4_mul_mat4(vect4_t v, mat4_t M);

#endif
