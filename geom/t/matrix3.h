#ifndef _matrix3_h_
#define _matrix3_h_

#ifdef __cplusplus 
extern "C" {
#endif

#include <stdio.h>
#include <math.h>
#define trig_sin(x)  sin(x)
#define trig_cos(x)  cos(x)

#define matSIZE 3

#define mat3_dump(A, str) mat3_fdump(NULL, (A), (str))

typedef struct {
	double a[matSIZE];
} vect3_t, *pvect3_t;

typedef struct {
	double a[matSIZE][matSIZE]; 
} mat3_t, *pmat3_t;

typedef enum {
	_x_ = 0,
	_y_ = 1,
	_z_ = 2,
} axis_t;

mat3_t mat3_null();
mat3_t mat3_ident();

void     mat3_fdump(FILE *f, mat3_t A, char *name);
mat3_t   mat3_add(mat3_t A, mat3_t B);
mat3_t   mat3_sub(mat3_t A, mat3_t B);
mat3_t   mat3_mul(mat3_t A, mat3_t B);
vect3_t  mat3_mul_vect3(mat3_t M, vect3_t v);
mat3_t   mat3_transpose(mat3_t A);
double   mat3_det(mat3_t A);
mat3_t   mat3_scale(double sx, double sy);
mat3_t   mat3_rotation(double angle);
mat3_t   mat3_translation(double dx, double dy);
mat3_t   mat3_projection(double k, double l, double m);

#ifdef __cplusplus 
}
#endif
#endif
