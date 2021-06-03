#ifndef _matrix5_h_
#define _matrix5_h_

#ifdef __cplusplus 
extern "C" {
#endif

#include <stdio.h>
#include <math.h>
#define trig_sin(x)  sin(x)
#define trig_cos(x)  cos(x)

#define matSIZE 5

#define mat5_dump(A, str) mat5_fdump(NULL, (A), (str))

typedef struct {
	double a[matSIZE];
} vect5_t, *pvect5_t;

typedef struct {
	double a[matSIZE][matSIZE]; 
} mat5_t, *pmat5_t;

typedef enum {
	_x0_ = 0,
	_x1_ = 1,
	_x2_ = 2,
	_x3_ = 3,
	_x4_ = 4,
} axis_t;

mat5_t mat5_null();
mat5_t mat5_ident();

void     mat5_fdump(FILE *f, mat5_t A, char *name);
mat5_t   mat5_add(mat5_t A, mat5_t B);
mat5_t   mat5_sub(mat5_t A, mat5_t B);
mat5_t   mat5_mul(mat5_t A, mat5_t B);
vect5_t  mat5_mul_vect5(mat5_t M, vect5_t v);
mat5_t   mat5_transpose(mat5_t A);
double   mat5_det(mat5_t A);
mat5_t   mat5_scale(double sx, double sy);
mat5_t   mat5_rotation(double angle);
mat5_t   mat5_translation(double dx, double dy);
mat5_t   mat5_projection(double k, double l, double m);

#ifdef __cplusplus 
}
#endif
#endif
