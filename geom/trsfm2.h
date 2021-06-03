#ifndef _trsfm2_h_
#define _trsfm2_h_

//#include <stdlib.h>
#include "geom_defs.h"
#ifndef _mat3_h_
#include "mat3.h"
#endif

#ifndef _trsfm2_c_
typedef void *ptrsfm2_t;
#endif

size_t trsfm2_sizeof();
ptrsfm2_t trsfm2_new();
ptrsfm2_t trsfm2_destroy(ptrsfm2_t p);

double trsfm2_dist(double x1, double y1, double x2, double y2) ;

int trsfm2_eye_set(ptrsfm2_t p, double r, double theta, double phy);
int trsfm2_pov_set(ptrsfm2_t p, double r, double theta, double phy);
int trsfm2_theta_set(ptrsfm2_t p, double theta);
int trsfm2_phy_set(ptrsfm2_t p, double phy);
pt2D_t trsfm2_transform(ptrsfm2_t p, pt2D_t pt);

#endif
