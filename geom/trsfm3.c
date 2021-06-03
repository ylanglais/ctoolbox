#include <stdlib.h>

enum { Tg, Tv, Trl, Ttheta, Tphy, To, Tp, Ts };

#include "mat4.h"

typedef struct {
	/* eye position: */
	struct {
		double r, theta, phy;
		double x, y, z;
	} eye;

	/* point of view position: */
	struct {
		double r, theta, phy;
		double x, y, z;
	} pov;

	/* Transformation matrix: */
	mat4_t tmat[8];

} trsfm3_t, *ptrsfm3_t;

#define _trsfm3_c_
#include "trsfm3.h"
#undef _trsfm3_c_

size_t trsfm3_sizeof() { return sizeof(trsfm3_t); }

ptrsfm3_t 
trsfm3_destroy(ptrsfm3_t p)  {
	if (p) free(p);
	return p;
}

trsfm3_t 
trsfm3_init() {
	int i;
	trsfm3_t t;

	t.eye.r = t.eye.theta = t.eye.phy = t.eye.x = t.eye.y = t.eye.z = 0.;
	t.pov.r = t.pov.theta = t.pov.phy = t.pov.x = t.pov.y = t.pov.z = 0.;

	for (i = 0; i < 8; i++) t.tmat[i] = mat4_ident();

	t.tmat[Tp] = mat4_projection(0, 0, t.eye.r / 10.);
	t.tmat[Ts] = mat4_scale(1./t.eye.r, 1./t.eye.r, 1./t.eye.r);
	/* t.tmat[Trl] = mat4_scale(1., 1., -1.); */	
	return t;
}

ptrsfm3_t
trsfm3_new() {
	int i;
	ptrsfm3_t p;

	if (!(p = (ptrsfm3_t) malloc(trsfm3_sizeof()))) return NULL;

	p->eye.r = p->eye.theta = p->eye.phy = p->eye.x = p->eye.y = p->eye.z = 0.;
	p->pov.r = p->pov.theta = p->pov.phy = p->pov.x = p->pov.y = p->pov.z = 0.;

	for (i = 0; i < 8; i++) p->tmat[i] = mat4_ident();

	p->tmat[Tp] = mat4_projection(0, 0, p->eye.r / 10.);
	p->tmat[Ts] = mat4_scale(1./p->eye.r, 1./p->eye.r, 1./p->eye.r);
	/* p->tmat[Trl] = mat4_scale(1., 1., -1.); */
	
	return p;
}

double
trsfm3_dist(double x1, double y1, double z1, double x2, double y2, double z2) {
	return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
}

int
trsfm3_eye_set(ptrsfm3_t p, double r, double theta, double phy) {
	if (!p) return 1;
	p->eye.r     = r;
	p->eye.theta = theta;
	p->eye.phy   = phy;
	
	p->eye.x = r * cos(theta) * cos(phy);;
	p->eye.y = r * sin(theta) * cos(phy);
	p->eye.z = r * sin(phy);

	p->tmat[To] = mat4_translation(0, 0, trsfm3_dist(p->eye.x, p->eye.y, p->eye.z, p->pov.x, p->pov.y, p->pov.z)); 
	p->tmat[Tp] = mat4_projection(0, 0, p->eye.r / 10.);
	p->tmat[Ts] = mat4_scale(1./p->eye.r, 1./p->eye.r, 1./p->eye.r);
	
	return 0;
}

int
trsfm3_pov_set(ptrsfm3_t p, double r, double theta, double phy) {
	if (!p) return 1;
	p->pov.r     = r;
	p->pov.theta = theta;
	p->pov.phy   = phy;
	
	p->pov.x = r * cos(theta) * cos(phy);;
	p->pov.y = r * sin(theta) * cos(phy);
	p->pov.z = r * sin(phy);

	p->tmat[Tv] = mat4_translation(-p->pov.x, -p->pov.y, -p->pov.z);
	p->tmat[To] = mat4_translation(0, 0, trsfm3_dist(p->eye.x, p->eye.y, p->eye.z, p->pov.x, p->pov.y, p->pov.z)); 
	
	return 0;
}

int
trsfm3_theta_set(ptrsfm3_t p, double theta) {
	if (!p) return 1;
	p->tmat[Ttheta] = mat4_rotation(_z_, theta);	
	return 0;
}

int trsfm3_phy_set(ptrsfm3_t p, double phy) {
	if (!p) return 1;
	p->tmat[Tphy] = mat4_rotation(_y_, phy);  
	return 0;
}

pt3D_t 
trsfm3_transform(ptrsfm3_t p, pt3D_t pt) {
	pt3D_t rpt = {0, 0, 0};
	vect4_t v;
	if (!p) return  rpt;

	v.a[_x_] = pt.x;
	v.a[_y_] = pt.y;
	v.a[_z_] = pt.z;
	v.a[_t_] = 1;

	/* Tg, Tv, Trl, Ttheta, Tphy, To, Tp */
	v = vect4_mul_mat4(v, p->tmat[Tv]);
	v = vect4_mul_mat4(v, p->tmat[Trl]);
	v = vect4_mul_mat4(v, p->tmat[Ttheta]);
	v = vect4_mul_mat4(v, p->tmat[Tphy]);
	v = vect4_mul_mat4(v, p->tmat[To]);
	v = vect4_mul_mat4(v, p->tmat[Tp]);
	v = vect4_mul_mat4(v, p->tmat[Ts]);

	rpt.x = v.a[_x_];
	rpt.y = v.a[_y_];
	rpt.z = v.a[_z_];
	return rpt;
}

#ifdef _test_trsfm3_

int main() {

	return 0;
}

#endif
