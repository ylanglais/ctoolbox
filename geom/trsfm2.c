#include <stdlib.h>
#include "mat3.h"

typedef mat3_t trsfm2_t, *ptrsfm2_t;

#define _trsfm2_c_
#include "trsfm2.h"
#undef _trsfm2_c_

size_t trsfm2_sizeof() { return sizeof(trsfm2_t); }

ptrsfm2_t 
trsfm2_destroy(ptrsfm2_t p)  {
	if (p) free(p);
	return p;
}

trsfm2_t 
trsfm2_init() {
	return mat3_ident();
}

ptrsfm2_t
trsfm2_new() {
	ptrsfm2_t p;

	if (!(p = (ptrsfm2_t) malloc(trsfm2_sizeof()))) return NULL;

	*p = mat3_ident();
	
	return p;
}

double
trsfm2_dist(double x1, double y1, double x2, double y2) {
	return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

pt2D_t 
trsfm2_transform(ptrsfm2_t p, pt2D_t pt) {
	pt2D_t rpt = {0, 0};
	vect3_t v;
	if (!p) return  rpt;

	v.a[_x_] = pt.x;
	v.a[_y_] = pt.y;
	v.a[_z_] = 1;

	v = mat3_mul_vect3(*(mat3_t *)p, v);

	rpt.x = v.a[_x_];
	rpt.y = v.a[_y_];
	return rpt;
}

#ifdef _test_trsfm2_

int main() {
	return 0;
}


#endif
