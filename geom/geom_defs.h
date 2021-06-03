#ifndef _geom_defs_h_
#define _geom_defs_h_

typedef struct {
	double x, y;
} pt_t, *ppt_t;

typedef pt_t pt2D_t, *ppt2D_t;

typedef struct {
	pt_t p0, p1;
} line_t, *pline_t;

typedef line_t line2D_t, *pline2D_t;

typedef struct {
	double x0, y0, x1, y1; 
} rect_t, *prect_t;

typedef rect_t rect2D_t, *prect2D_t;

typedef struct {
	double x, y, z;
} pt3D_t, *ppt3D_t;

typedef struct {
	pt3D_t p0, p1;
} line3D_t, *pline3D_t;

#endif
