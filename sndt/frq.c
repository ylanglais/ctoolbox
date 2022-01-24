#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <tbx/err.h>
#include <tbx/mem.h>
#include <tbx/dyna.h>
#include <tbx/thpo.h>

#include "ampl.h"

double _frqs_[] = {
/*             A        A#/Bb      B         C        C#/Db      D        D#/Eb      E         F        F#/Gb      G       G#/Ab   */
/*  0 */	   13.75,    14.57,    15.43,    16.35,    17.32,    18.35,    19.45,    20.60,    21.83,    23.12,    24.50,    25.96,
/*  1 */	   27.50,    29.14,    30.87,    32.70,    34.65,    36.71,    38.89,    41.20,    43.65,    46.25,    49.00,    51.91,
/*  2 */	   55.00,    58.27,    61.74,    65.41,    69.30,    73.42,    77.78,    82.41,    87.31,    92.50,    98.00,   103.83,
/*  3 */	  110.00,   116.54,   123.47,   130.81,   138.59,   146.83,   155.56,   164.81,   174.61,   185.00,   196.00,   207.65,
/*  4 */	  220.00,   233.08,   246.94,   261.63,   277.18,   293.66,   311.13,   329.63,   349.23,   369.99,   392.00,   415.30,
/*  5 */	  440.00,   466.16,   493.88,   523.25,   554.37,   587.33,   622.25,   659.26,   698.46,   739.99,   783.99,   830.61,
/*  6 */	  880.00,   932.33,   987.77,  1046.50,  1108.73,  1174.66,  1244.51,  1318.51,  1396.91,  1479.98,  1567.98,  1661.22,
/*  7 */	 1760.00,  1864.66,  1975.53,  2093.00,  2217.46,  2349.32,  2489.02,  2637.02,  2793.83,  2959.96,  3135.96,  3322.44,
/*  8 */	 3520.00,  3729.31,  3951.07,  4186.01,  4434.92,  4698.64,  4978.03,  5274.04,  5587.65,  5919.91,  6271.93,  6644.88,
/*  9 */	 7040.00,  7458.62,  7902.13,  8372.02,  8869.84,  9397.27,  9956.06, 10548.08, 11175.30, 11839.82, 12543.85, 13289.75,
///* 10 */	14080.00, 14917.24, 15804.27, 16744.04, 17739.69, 18794.55, 19912.13, 21096.16, 22350.61, 23679.64, 25087.71, 26579.50,
};

#define _nfrqs_  (sizeof(_frqs_) / sizeof(double))

#define _frq_wdt_ .1
#define _frq_wlo_ 1

static char _frq_file_sign[] = "-->Freq--File<--";

typedef struct {
	int    samples;
	double duration;
	double tunit;
	double *frqs;
} frq_t, *pfrq_t;

#define _frq_c_
#include "frq.h"
#undef  _frq_c_

int
frq_nfreqs() {
	return _nfrqs_;
}

double
frq_frequency(int i) {
	if (i < 0 || i >= _nfrqs_) return 0.;
	return _frqs_[i];
}

/* 
 * mft accuracy varies w/ dt and frequency: 
 * dt:
 *    A small dt increases time resolution (note start/stop), but lowers    frequency resolution.
 *    A big   dt lowers    time resolution (note start/stop), but increases frequency resolution.   
 * 
 * Frequency:
 *    On low frquencies, dt becomes too big to get a good frequency resolution.
 *    => A specific low pass processing should improve low frquencies resolution.
 * 
 * mft only uses frquencies corresponding to actual notes for decomposition. 
 * Instrument tuning has an influence on frequency acuracy: Untuned notes will widely spread on different note frquencies. 
 * Tuning deviation can be systematic (instrument wide tuning), recurrent (guitar strings), or notewise (a piano).
 *
 */

int
frq_dtf(double t0, double dt, int nsamples, double *A, double *f) {
	static double m2PI = 2. * M_PI;
    register int i, k;
    register double arg, karg;
	register double *x, *y;
	
    if (!(x  = (double *) malloc(_nfrqs_ * sizeof(double)))) {
		return 2;
	}
    if (!(y  = (double *) malloc(_nfrqs_ * sizeof(double)))) {
		free(x);
		return 3;
	}
err_info("%d samples @%p", nsamples, A);

	// double max = 0;
    for (i = 0; i < _nfrqs_; i++) {
        x[i] = 0;
        y[i] = 0;

		double t;
        arg =  m2PI * _frqs_[i];
        for (k = 0, t = t0; k < nsamples; k++, t += dt) {
			karg = t * arg; 
			/* y1 part = 0 */
            x[i] +=   A[k] * cos(karg); /* + y1[k] * sin(karg); */
            y[i] += - A[k] * sin(karg); /* + y1[k] * cos(karg); */
			/*
			if (k == nsamples - 1) {
				double M = x[i] * x[i] + y[i] * y[i]);
				if ( M > max) max = M;
			}
			*/
        }
		/* Normalize: */
		x[i] *= 2. / (double) nsamples; 
		y[i] *= 2. / (double) nsamples;
    }

	for (i = 0; i < _nfrqs_; i++) {
		f[i] = sqrt(x[i] * x[i] + y[i] * y[i]);	
	}
	free(x); free(y);
    return 0;
}

static double _frq_max(double a, double b) { if (a > b) return a; else return b; }
static double _frq_min(double a, double b) { if (a < b) return a; else return b; }

pfrq_t 
frq_new(pampl_t a, void (*progress_cb)(void *, double), void *data) {
	pfrq_t frq;
	if (!a) return NULL;

	if (!(frq = (pfrq_t) mem_zmalloc(sizeof(frq_t)))) {
		return NULL;
	}

	/* 
	 * 	Normal scale is 1000px for 10s => 1px is 0.01s 
	 *	time per pixel: 
     *     tpp = .01
	 * 
	 *  w/ 44100 Hz sampling rate => 441 samples:
	 * 
	 *  number of samples per pixel:
     *     nspp  =  sr * tpp;
     *
	 *  need ~ 10000 samples to get a correct sampling
	 *	   ftns = 10000;
     *
	 * 
	 *  For pixel i of 0.01s duratition, starting at sample i * 441
	 *	  => start at max(0,        (i * nspp - (ftns - nspp) / 2)
	 * 	  => end   at min(nsamples, (i * nspp + (ftns - nspp) / 2);
	 *
	 */

	//double tpp  =    0.01; /* 10s = 1000px => .01s per pixel */
	double tpp  =    0.05; 
	int    ftns = 4410; 

	double t0;
	int    pix;

	double *A   = ampl_amplitudes(a);
	double dt   = ampl_tunit(a);
	int    ns   = ampl_samples(a);
	int    sr   = ampl_samplerate(a);
	int    npix = ampl_duration(a) / tpp;
	int    nspp = (int) ((double) sr * tpp);
	int    delt = (ftns - nspp) / 2;

	if (!(frq->frqs = malloc(_nfrqs_ * npix * sizeof(double)))) {
		return frq_destroy(frq);
	}

	for (pix = 0, t0 = 0; pix < npix; pix++, t0 += tpp) {
		int i    = _frq_max(0,  pix * nspp - delt);
		int n    = _frq_min(ns, pix * nspp + delt);
		int win  = n - i;
		frq_dtf(t0, dt, win, A + i, frq->frqs + (pix * _nfrqs_));
		if (progress_cb) progress_cb(data, (double) pix / (double) npix * 100.);
	}	
	frq->samples  = npix;
	frq->tunit    = 0.05;
	frq->duration = ampl_duration(a);
	
	return frq;
}
	
/*  convert (t,tunit A) data into (t, F) data */

pfrq_t
frq_win_new(pampl_t a, double twin, void (*progress_cb)(void *, double), void *data) {
	pfrq_t frq;


	if (!a || twin <= 0) return NULL;

	if (!(frq = (pfrq_t) mem_zmalloc(sizeof(frq_t)))) {
		return NULL;
	}

	int n = ampl_samples(a);

	if (!(frq->frqs = malloc(_nfrqs_ * n * sizeof(double)))) {
		return frq_destroy(frq);
	}
	
	int i, j;
	double t0;
	int win;

	win = twin * ampl_samplerate(a);

	for (j = i = 0, t0 = 0; i < n - win; j++, i += win, t0 += twin) {
		if (i + win > n - 1) win = n - 1 - i;
		frq_dtf(t0, ampl_tunit(a), win, ampl_amplitudes(a) + i , frq->frqs + (j * _nfrqs_));
		//printf("%3d: from %d to %d (from %lf to %lf) & copy to %d\n", j, i, i + win, t0, t0 + twin, (int) (j * _nfrqs_)); 
		if (progress_cb) progress_cb(data, (t0/ampl_duration(a)) * 100.);
	}

	frq->samples  = j;
	frq->tunit    = twin;
	frq->duration = ampl_duration(a);
	
	return frq;
}

typedef struct _tdfs_t {
	double 	t0; 
	double 	dt; 
	int 	nsamples; 
	double 	*A; 
	double 	*f;
	void (*progress_cb)(void *);
	void *data;
} dtfs_t, *pdtfs_t;

int
frq_dtfs(void * data) {
	static double m2PI = 2. * M_PI;
    register int i, k;
    register double arg, karg;
	register double *x, *y;
	pdtfs_t dtfs;
	if (!data) return 1;
	dtfs = (pdtfs_t) data;
	
    if (!(x  = (double *) malloc(_nfrqs_ * sizeof(double)))) {
		return 2;
	}
    if (!(y  = (double *) malloc(_nfrqs_ * sizeof(double)))) {
		free(x);
		return 3;
	}
	
err_info("%d samples @%p", dtfs->nsamples, dtfs->A);

	double max = 0;
    for (i = 0; i < _nfrqs_; i++) {
        x[i] = 0;
        y[i] = 0;

		double t;
        arg =  m2PI * _frqs_[i];
        for (k = 0, t = dtfs->t0; k < dtfs->nsamples; k++, t += dtfs->dt) {
			karg = t * arg; 
			/* y1 part = 0 */
            x[i] +=   dtfs->A[k] * cos(karg); /* + y1[k] * sin(karg); */
            y[i] += - dtfs->A[k] * sin(karg); /* + y1[k] * cos(karg); */
			if (k == dtfs->nsamples - 1) {
				double M = sqrt(x[i] * x[i] + y[i] * y[i]);
				if ( M > max) max = M;
			}
        }
		/* Normalize: */
		x[i] *= 2. / (double) dtfs->nsamples; 
		y[i] *= 2. / (double) dtfs->nsamples;
    }

	for (i = 0; i < _nfrqs_; i++) {
		dtfs->f[i] = sqrt(x[i] * x[i] + y[i] * y[i]);	
	}
	free(x); free(y);
	if (dtfs->progress_cb) dtfs->progress_cb(dtfs->data);

    return 0;
}

pfrq_t 
frq_new_thpo(pampl_t a, int nthreads, void (*progress_cb)(void *), void *data) {
	pfrq_t frq;
	if (!a) return NULL;

	if (!(frq = (pfrq_t) mem_zmalloc(sizeof(frq_t)))) {
		return NULL;
	}

	pdyna_t da;

	if (!(da = dyna_new(sizeof(dtfs_t), 1000))) return frq_destroy(frq);

	/* 
	 * 	Normal scale is 1000px for 10s => 1px is 0.01s 
	 *	time per pixel: 
     *     tpp = .01
	 * 
	 *  w/ 44100 Hz sampling rate => 441 samples:
	 * 
	 *  number of samples per pixel:
     *     nspp  =  sr * tpp;
     *
	 *  need ~ 10000 samples to get a correct sampling
	 *	   ftns = 10000;
     *
	 * 
	 *  For pixel i of 0.01s duratition, starting at sample i * 441
	 *	  => start at max(0,        (i * nspp - (ftns - nspp) / 2)
	 * 	  => end   at min(nsamples, (i * nspp + (ftns - nspp) / 2);
	 *
	 */

	//double tpp  =     0.01; /* 10s = 1000px => .01s per pixel */
	double tpp  =     0.05; 
	int    ftns = 4410; 

	double t0;
	int    pix;

	double *A   = ampl_amplitudes(a);
	double dt   = ampl_tunit(a);
	int    ns   = ampl_samples(a);
	int    sr   = ampl_samplerate(a);
	int    npix = ampl_duration(a) / tpp;
	int    nspp = (int) ((double) sr * tpp);
	int    delt = (ftns - nspp) / 2;

	char *p;

	if (!(frq->frqs = malloc(_nfrqs_ * npix * sizeof(double)))) return frq_destroy(frq);
	
	pthpo_t tp;
	if (!(tp = thpo_new(nthreads, (int (*)(void *)) frq_dtfs))) return frq_destroy(frq);

	thpo_run(tp);

	for (pix = 0, t0 = 0; pix < npix; pix++, t0 += tpp) {
		int i    = _frq_max(0,  pix * nspp - delt);
		int n    = _frq_min(ns, pix * nspp + delt);
		int win  = n - i;
		//frq_dtf(t0, dt, win, A + i , frq->frqs + (pix * _nfrqs_));

		dtfs_t d;
		d.t0          = t0;
		d.dt          = dt; 
		d.nsamples    = win;
		d.A           = A + i; 
		d.f           = frq->frqs + (pix * _nfrqs_);
		d.progress_cb = progress_cb;
		d.data        = data;

		p = dyna_push(da, (char *) &d);
		thpo_push(tp, pix, p);
	}	
	thpo_done(tp);
	if (getenv("THPOSTATS"))  thpo_stats_dump(tp);
	if (getenv("THPODETAIL")) thpo_stats_detail(tp);
	thpo_destroy(tp);
	dyna_destroy(da);
	frq->samples  = npix;
	frq->tunit    = 0.05;
	frq->duration = ampl_duration(a);
	
	return frq;
}

pfrq_t
frq_destroy(pfrq_t frq) {
	if (frq) {
		if (frq->frqs) free(frq->frqs);
		free(frq);	
	} 
	return NULL;
}

double 
frq_get(pfrq_t frq, int i, int j) {
	if (!frq) {
		return -1;
	}
	if (!frq->frqs) {	
		return -2;
	}
	if (i < 0 || i >= frq->samples) {
		return -3;
	}
	if (j < 0 || j >= _nfrqs_) {
		return -4;
	}
	return frq->frqs[i * _nfrqs_ + j];
}

double
frq_at(pfrq_t frq, double time, int j) {
	if (!frq) {
		return -1;
	}
	if (!frq->frqs) {	
		return -2;
	}
	if (frq->tunit == 0.) {
		return -4;
	}
	if (j < 0 || j >= _nfrqs_) {
		return -4;
	}
	return frq_get(frq, (int) (time / frq->tunit), j);
}

int 
frq_samples(pfrq_t frq) {
	if (!frq) {
		return -1;
	}
	return frq->samples;
}

double 
frq_tunit(pfrq_t frq) {
	if (!frq) {
		return -1;
	}
	return frq->tunit;
}

double 
frq_duration(pfrq_t frq) {
	if (!frq) {
		return 0;
	} 
	return frq->duration;
}

double *
frq_ptr(pfrq_t frq) {
	if (!frq) {
		return NULL;
	}
	return frq->frqs;
}

int
frq_save(pfrq_t frq, char *fname) {
	int    f;
	size_t s;

	if ((f = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) { 
		err_error("cannot open %s : %s\n", fname, strerror(errno));
		return 1;
	}
	if ((s = write(f, _frq_file_sign, strlen(_frq_file_sign))) < strlen(_frq_file_sign)) {
		err_error("partial writing: cannot write file type signature");
		close(f);
		return 2;
	}
	if ((s = write(f, &(frq->samples), sizeof(int))) != sizeof(int)) {
		err_error("partial writing: cannot write sample nuber");
		close(f);
		return 3;
	}
	if ((s = write(f, &(frq->tunit), sizeof(double))) != sizeof(double)) {
		err_error("partial writing: cannot write time unit");
		close(f);
		return 4;
	}
	if ((s = write(f, frq->frqs, _nfrqs_ * frq->samples * sizeof(double))) != _nfrqs_ * frq->samples * sizeof(double)) {
		err_error("partial writing: cannot frequency samples");
		close(f);
		return 5;
	}
	close(f);
	return 0;	
}

pfrq_t 
frq_load(char *fname) {
	pfrq_t frq;
	int f;
	char sign[100];
		
	if ((f = open(fname, O_RDONLY)) < 0) { 
		err_error("cannot open %s : %s\n", fname, strerror(errno));
		return NULL;
	}
	if (!(frq = (pfrq_t) mem_zmalloc(sizeof(frq_t)))) {
		err_error("cannot allocate memory for frq structure");
		close(f);
		return NULL;
	}
	if (!read(f, sign, strlen(_frq_file_sign))) {
		err_error("error reading signature: %s\n", strerror(errno));
		close(f);
		return frq_destroy(frq);
	}	
	if (strcmp(_frq_file_sign, sign)) {
		err_error("invalid frq file signature");
		close(f);
		return frq_destroy(frq);
	}
	if (!read(f, &(frq->samples), sizeof(int))) {
		err_error("error reading frequency sample count: %s", strerror(errno));
		close(f);
		return frq_destroy(frq);
	}
	if (!read(f, &(frq->tunit), sizeof(double))) {
		err_error("error reading time unit: %s", strerror(errno));
		close(f);
		return frq_destroy(frq);
	}
	if (!(frq->frqs = (double *) malloc(frq->samples * _nfrqs_ * sizeof(double)))) {
		err_error("error allocating frequency samples");
		return frq_destroy(frq);
	}
	if (!read(f, frq->frqs, _nfrqs_ * frq->samples * sizeof(double))) {
		err_error("error reading frequency samples: %s", strerror(errno));
		close(f);
		return frq_destroy(frq);
	}
	close(f);
	return frq;	
}

void
frq_freq_dump(pfrq_t frq, int i, char *fname) {
	int j;
	FILE *F = NULL;

	if (!frq) return;
	if (i < 0 || i > frq->samples) 
		return;
	
	if (fname && *fname) {
		F = fopen(fname, "w");
	} 
	if (!F) F = stdout;

	for (j = 0; j < _nfrqs_; j++) {
		fprintf(F, "%d\t%lf\t%lf\n", j, _frqs_[j], frq->frqs[i * _nfrqs_ + j]);
	}
	if (F != stdout) fclose(F);
}

void
frq_dump(pfrq_t frq, char *fname) {
	int i, j;
	double t;
	FILE *F = NULL;
	
	if (!frq) return;
	if (fname && *fname) {
		F = fopen(fname, "w");
	} 
	if (!F) F = stdout;

	for (i = 0, t = 0; i < frq->samples; i++, t += frq->tunit) {
		fprintf(F, "%d\t%lf", i, t);
		for (j = 0; j < _nfrqs_; j++) {
			fprintf(F, "\t%lf", frq->frqs[i * _nfrqs_ + j]);
		}
		fprintf(F, "\n");
	}	
	if (F != stdout) fclose(F);
}
