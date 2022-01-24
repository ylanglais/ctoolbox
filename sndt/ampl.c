#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sndfile.h>
#include <math.h>

#include <tbx/err.h>
#include <tbx/mem.h>

#include "wav.h"

typedef unsigned long ulong_t;

typedef struct {
	double   duration;    /* duration in ms       */
	ulong_t  samplerate;  /* nb of sampes per sec */
	ulong_t  samples;
	double  *amplitude;  /* */
} ampl_t, *pampl_t;

#define  _ampl_c_ 
#include "ampl.h" 
#undef  _ampl_c_ 

static int
ampl_min(int x, int y) {
	if (x <= y) return x;
	return y;
}

static int
ampl_max(int x, int y) {
	if (x >= y) return x;
	return y;
}

pampl_t 
ampl_new(double duration /* s */, ulong_t samplerate /* n/s */) {
	pampl_t a;

	if (!(a = (pampl_t) malloc(sizeof(ampl_t)))) {
		return NULL;
	}

	a->samples = duration * samplerate;
	if (!(a->amplitude = (double *) mem_zmalloc(a->samples * sizeof(double)))) {
		return ampl_destroy(a);
	}	
	a->duration   = duration; 
	a->samplerate = samplerate;
	return a;
}

pampl_t
ampl_destroy(pampl_t a) {
	if (a) {
		if (a->amplitude) {
			free(a->amplitude);
			a->amplitude = NULL;
		}
		free(a);
	}
	return NULL;
}

int
ampl_wav_save(pampl_t a, char *fname) {
    SF_INFO  info;
	SNDFILE *wav;

	if (!a)            return 1;
	if (!a->amplitude) return 2;
	if (!a->samples)   return 3;

    info.format     = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    info.channels   = 1;
    info.samplerate = a->samplerate;

    if (!(wav = sf_open(fname, SFM_WRITE, &info))) {
        err_error("cannot open sound file '%s': %s", fname, sf_strerror(wav));
        return 4;
    }

    /* Write frames: */
    if ((sf_writef_double(wav, a->amplitude, a->samples)) != a->samples) {
        err_error("problem writintg frames");
        sf_close(wav);
        return 5;
    }

    /* Terminate wav writing and close: */
    sf_write_sync(wav);
    sf_close(wav);

	return 0;	
}

pampl_t
ampl_wav_load(char *fname, int channel) {
	pwav_t wav;
	if (!(wav = wav_new(fname))) {
		return NULL;
	}
	pampl_t a = ampl_from_wav(wav, channel);
	wav_destroy(wav);
	return a;
}

pampl_t
ampl_from_wav(pwav_t wav, int channel) {
	pampl_t a;
	if (!wav) return NULL;	
	if (wav_channels(wav) < channel) {
		err_error("not enough channels (only %d, asked %d)", wav_channels(wav), channel);
		return NULL;
	}
	if (!(a = ampl_new(wav_length(wav), wav_samplerate(wav)))) {
		return NULL;
	}	
	memcpy(a->amplitude, wav_amplitude_array(wav, channel), a->samples * sizeof(double));
	ampl_normalize(a);
	return a;
}

pampl_t
ampl_dup(pampl_t a) {
	pampl_t b;

	if (!a) return NULL;
	
	b = ampl_new(a->duration, a->samplerate);
	memcpy(b->amplitude, a->amplitude, a->samples * sizeof(double));

	return b;
}

int
ampl_remove(pampl_t a, double from, double to) {
	if (!a)         return 1;
	if (to   < 0)   return 2;
	if (from >= a->duration)
				    return 3;
	if (from >= to) return 4;
	if (from < 0) from = 0;
	if (from == 0. && to >= a->duration)
				    return 5;	

	int i0 = from * a->samplerate;
	int i1 = to   * a->samplerate;
	
	if (i1 > a->samples) i1 = a->samples - 1;

	if (i1 < a->samples - 1) {
		memmove(a->amplitude + i0, a->amplitude + i1, a->samples - i1);
	}
	
	double *data;
	if (!(data = realloc(a->amplitude, (a->samples - i1 + i0) * sizeof(double)))) {
		err_error("cannot realloc !!!");
		return 6;
	}
	if (data != a->amplitude) a->amplitude = data;
	
	a->samples -= i1 - i0;
	a->duration = (double) a->samples / (double) a->samplerate;
	
	return 0;
}

int
ampl_blank(pampl_t a, double from, double to) {
	if (!a)                  return 1;
	if (to < 0)              return 2;
	if (from >= a->duration) return 3;

	if (from < 0) 
		from = 0;
	
	if (to > a->duration) 
		to = a->duration;
	
	int i0 = from * a->samplerate;
	int i1 = to   * a->samplerate;
	if (i1 >= a->samples) i1 = a->samples;

	memset(a->amplitude + i0, 0, (i1 - i0) * sizeof(double));
	return 0;
}


pampl_t
ampl_cut(pampl_t a, double from, double to) {
	pampl_t b;
	if (!(b = ampl_copy(a, from, to))) return NULL;
	if (ampl_blank(a, from, to)) {
		return ampl_destroy(b);
	}
	return b;
}

pampl_t
ampl_copy(pampl_t a, double from, double to) {
	pampl_t b;

	if (!a) return NULL;
	if (from < 0) 
		err_error("invalid from");	
	else if (to < 0) 
		err_error("invalid to");	
	else if (to <= from) 
		err_error("from is after to");	
	else if (from > ampl_duration(a)) 
		err_error("from starts too late");
	else if (to > ampl_duration(a)) 
		to = ampl_duration(a);
		
	b = ampl_new(to - from, a->samplerate);

	int i0;
	i0 = from * a->samplerate;

	memcpy(b->amplitude, a->amplitude + i0, b->samples * sizeof(double));
	return b;
}

int
ampl_paste(pampl_t dst, double at, pampl_t src) {
	if (!src)               return 1;
	if (!dst)               return 2;
	if (at < 0)             return 3;
	if (at > dst->duration) return 4;

	pampl_t s;
	if (src->samplerate != dst->samplerate) {
		if (!(s = ampl_resample(src, dst->samplerate))) 
                            return 5;
	} else {
		s = src;
	}

	int from = dst->samplerate *  at;
	int nsam = s->samples;


	if (nsam + from > dst->samples) 
		nsam = dst->samples - from;

	memcpy(dst->amplitude + from, s->amplitude, nsam * sizeof(double));

	if (s != src) ampl_destroy(s);

	return 0;
}

int
ampl_insert_blank(pampl_t a, double at, double duration) {
	if (!a) 	       return 1;
	if (duration <= 0) return 2;

	if (at < 0)           at = 0;
	if (at > a->duration) at = a->duration;
	
	double *data;
		
	int	    nsam = duration * a->samplerate;
	if (!(data = (double *) realloc(a->amplitude, (a->samples + nsam) * sizeof(double)))) {
		return 3;
	} 
	if (data != a->amplitude) a->amplitude = data;
	
	int i0 = a->samplerate * at;
	memmove(a->amplitude + i0, a->amplitude + i0 + nsam,  a->samples - i0);
	a->samples  += nsam;
	a->duration  = a->samples / a->samplerate; 
	return 0;
}

int
ampl_insert(pampl_t dst, double at, pampl_t src) {
	int r;
	if (src) return 1;
	if ((r = ampl_insert_blank(dst, at, src->duration))) {
		return r;
	}
	return ampl_paste(dst, at, src);
}

int
ampl_append(pampl_t a, pampl_t src) {
	if (!src) return 1;
	int r;
	if ((r = ampl_append_blank(a, src->duration))) {
		return r;
	}
	return ampl_paste(a, src->duration, src);
}

int
ampl_append_blank(pampl_t a, double duration) {
	if (!a) return 1;	
	if (duration <= 0) return 2;
	
	int	    nsam = duration * a->samplerate;
	double *data;
	if (!(data = (double *) realloc(a->amplitude, (a->samples + nsam) * sizeof(double)))) {
		return 3;
	} 
	if (data != a->amplitude) a->amplitude = data;

	memset(a->amplitude + a->samples, 0, nsam * sizeof(double));
	
	a->samples  += nsam;
	a->duration  = a->samples / a->samplerate; 
	return 0;
}

int
ampl_normalize(pampl_t a) {
	double aa;
	double max = 0.;

	if (!a)            return 1;
	if (!a->samples)   return 2;
	if (!a->amplitude) return 3;

	for (int i = 0; i < a->samples; i++) {
		aa = fabs(a->amplitude[i]);
		if (aa > max) max = aa;
	}
	if (max == 0. || max == 1.) return 4;	

	return ampl_div(a, max);
}

ulong_t
ampl_samples(pampl_t a) {
	if (!a) return 0;
	return a->samples;
}

ulong_t
ampl_samplerate(pampl_t a) {
	if (!a) return 0;
	return a->samplerate;
}

double
ampl_tunit(pampl_t a) {
	if (!a) return 0;
	return a->duration / a->samples;
}

double 
ampl_duration(pampl_t a) {
	if (!a) return 0;
	return a->duration;
}

double *
ampl_amplitudes(pampl_t a) {
	if (!a) return NULL;
	return a->amplitude;
}

double 
ampl_amplitude_at_t(pampl_t a, double t) {
	if (!a)            return amplDOUBLE_ERROR;
	if (!a->samples)   return amplDOUBLE_ERROR;
	if (!a->amplitude) return amplDOUBLE_ERROR;
	
	int i = rint(t * (double) a->samplerate);
	if (i < 0 || i > a->samples) return amplDOUBLE_ERROR;
	return a->amplitude[i];
}

pampl_t
ampl_resample(pampl_t a, ulong_t samplerate) {
	if (!a)            return NULL;
	if (!a->samples)   return NULL;
	if (!a->amplitude) return NULL;
	if (samplerate    == 0)
					   return NULL;
	if (a->samplerate == samplerate) 
					   return NULL;

	pampl_t b;
	if (!(b = ampl_new(a->duration, samplerate))) {
		return NULL;
	}

	int i; 
	double t, dt; 
	dt = a->duration / (double) b->samples; 
	for (i = 0, t = 0.; i < b->samples; i++, t += dt) {
		b->amplitude[i] = ampl_amplitude_at_t(a, t);
	}
	return b;
}

static int 
_ampl_op(pampl_t a, pampl_t b, amplOP op, double at_a, double at_b, double duration) {
	int i0, i1, i;
	double t, dt;

	if (!a)                  return 1;
	if (!b)                  return 2;
	if (at_a >= a->duration) return 3;
	if (at_b >= b->duration) return 4;
	if (at_b >= a->duration) return 5;

	if (op < ampl_opADD || op > ampl_opSUB) 
			                 return 6;

	i0 = ampl_min(ampl_max(0, rint( at_a             * (double) a->samplerate)), a->samples - 1);
	i1 = ampl_min(ampl_max(0, rint((at_a + duration) * (double) a->samplerate)), a->samples - 1);

	dt = a->duration / (double) a->samples;
		
	for (i = i0, t = at_b; i < i1 && t <= b->duration; i++, t += dt) {
		double aa = ampl_amplitude_at_t(b, t);
		if (op == ampl_opADD) 
			a->amplitude[i] += aa;
		else 
			a->amplitude[i] -= aa;
	}
	return 0;
}

int
ampl_add(pampl_t a, pampl_t b, double at_a, double at_b, double duration) {
	return _ampl_op(a, b, ampl_opADD, at_a, at_b, duration);
}

int
ampl_sub(pampl_t a, pampl_t b, double at_a, double at_b, double duration) {
	return _ampl_op(a, b, ampl_opSUB, at_a, at_b, duration);
}

int
ampl_mul(pampl_t a, double k) {
	if (!a)            return 1;
	if (!a->samples)   return 2;
	if (!a->amplitude) return 3;
	for (int i = 0; i < a->samples; i++) {
		a->amplitude[i] *= k;
	}
	return 0;
}

int
ampl_div(pampl_t a, double k) {
	if (k == 0.) {
		err_error("cannot divide by 0");
		return 1;
	}
	return ampl_mul(a, 1. / k);
}

pampl_t 
ampl_conv(pampl_t a, pampl_t b) {
	return NULL;
}

pampl_t 
ampl_deconv(pampl_t a, pampl_t b) {
	return NULL;
}

int
ampl_gen(pampl_t a, amplOP op, double amplitude, double fromtime, double totime, double freq, double phase) {
    static double m2PI = 2. * M_PI;
	int i, i0, i1;

	if (!a)                        return 1;
	if (!a->samples)               return 2;
	if (!a->amplitude)             return 2;
	if (fromtime >= totime)        return 4;
	if (op < ampl_opSET || op >= ampl_opBAD) 
								   return 5;

	i0 = ampl_min(ampl_max(0, rint(fromtime * (double) a->samplerate)), a->samples);
	i1 = ampl_min(ampl_max(0, rint(totime   * (double) a->samplerate)), a->samples);

	for (i = i0; i < i1; i++) {
		double t, aa;
		t = (double) i / (double) a->samplerate;
		aa = amplitude * sin(m2PI * freq * t + phase);
		switch (op) {
		case ampl_opSET:
			a->amplitude[i]  = aa;
			break;
		case ampl_opADD:
			a->amplitude[i] += aa;
			break;
		case ampl_opSUB:
			a->amplitude[i] -= aa;
			break;
		case ampl_opMUL:
			a->amplitude[i] *= aa;
			break;
		case ampl_opDIV:
			a->amplitude[i] /= aa;
			break;
		}
	}
	return 0;
}

void 
ampl_dump(pampl_t a, char *fname) {
	FILE *f = stdout;
	
	if (!a || !a->amplitude || !a->samples) return;

	if (fname && *fname) 
		if (!(f = fopen(fname, "w+"))) 
			f = stdout;
	int i;
	double t, dt;
	dt = a->duration / (double) a->samples;	
	fprintf(f, "#idx\tt(ms)\tamplitude\n");
	for (i = 0, t = 0.; i < a->samples; i++, t += dt) {
		fprintf(f, "%d\t%lf\t%lf\n", i, t, a->amplitude[i]);
	}

	if (f != stdout) fclose(f);
}

#ifdef _test_ampl_

int main(void) {
	pampl_t a, b; 

	const double A0 = 13.75;

	double fq[] = { 
		131.,
		147.,
		165., 
		175., 
		196.,
		220.,
		247.,
		262.,
		0.
	};

	a = ampl_new(10, 44100);


	int i;
	for (i = 0; i < 8; i++) {
		ampl_gen(a, ampl_opSET, 1., i, (i + 1), fq[i], 0); 
	}	

	#if 0
	ampl_dump(b, "test.b.ampl");
	double t, dt;
	dt = a->duration / (double) (a->samples);
	t = 0;
	for (i = 0; i < a->samples; i++, t += dt) {
		if (a->amplitude[i] != ampl_amplitude_at_t(a, t))
			printf("----> %d %lf %lf %lf\n", i, t, a->amplitude[i], ampl_amplitude_at_t(a, t));
	}
	#endif
	ampl_dump(a, "test.a.ampl");

	b = ampl_new(10, 44100);
	ampl_gen(b, ampl_opSET, 1., 0., 1., fq[0], 0);
	ampl_gen(b, ampl_opSET, 1., 2., 3., fq[2], 0);
	ampl_gen(b, ampl_opSET, .5, 4., 5., fq[4], 0);
	ampl_gen(b, ampl_opSET, 1., 6., 7., fq[6]/2., 0);

	ampl_dump(b, "test.b.ampl");
	ampl_sub(a, b, 0, 0, 10);

	ampl_normalize(a);
	ampl_dump(a, "test.a2.ampl");
	ampl_destroy(a);

	printf("\nC scale:\n");
	a = ampl_new(8.1, 44100);

	int shift[] = { 2, 2, 1, 2, 2, 2, 1 }; 
	int j;
	for (i = 0, j = 0; i < 8; j += shift[i++ % 7]) {
		double f = A0 * pow(2., (double) (j + 51) / 12.);
		printf("from %lf to %lf w/ %lf\n", (double) i, (double) (i+1), f); 
		ampl_gen(a, ampl_opSET, 1., (double) i, (double) (i + 1), f, 0);

	} 
	ampl_dump(a, "C.ampl");
	ampl_wav_save(a, "C.wav");
	ampl_destroy(a);

	const int count = 4 * 7;

	printf("\nC all:\n");
	a = ampl_new(count + 1, 44100);
	for (i = 0, j = 0; i <= count; j += shift[i++ % 7]) {
		double f = A0 * pow(2., (double) (j + 51) / 12.);
		printf("from %lf to %lf w/ %lf\n", (double) i, (double) (i+1), f); 
		ampl_gen(a, ampl_opSET, 1., (double) i , (double) (i + 1), f, 0);
	} 
	ampl_wav_save(a, "Call.wav");
	ampl_destroy(a);


	return 0;
}

#endif
