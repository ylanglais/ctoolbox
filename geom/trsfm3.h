#ifndef _trsfm3_h_
#define _trsfm3_h_

//#include <stdlib.h>
#include "geom_defs.h"
#ifndef _matrix4_h_
#include "matrix4.h"
#endif

#ifndef _trsfm3_c_
typedef void *ptrsfm3_t;
#endif

size_t trsfm3_sizeof();
ptrsfm3_t trsfm3_new();
ptrsfm3_t trsfm3_destroy(ptrsfm3_t p);

double trsfm3_dist(double x1, double y1, double z1, double x2, double y2, double z2) ;

int trsfm3_eye_set(ptrsfm3_t p, double r, double theta, double phy);
int trsfm3_pov_set(ptrsfm3_t p, double r, double theta, double phy);
int trsfm3_theta_set(ptrsfm3_t p, double theta);
int trsfm3_phy_set(ptrsfm3_t p, double phy);
pt3D_t trsfm3_transform(ptrsfm3_t p, pt3D_t pt);

#endif
