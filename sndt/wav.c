#include <stdlib.h>
#include <stdio.h>
#include <sndfile.h>
#include <string.h>

#include <tbx/mem.h>
#include <tbx/err.h>

typedef struct {
	int    frames; 
	int    samplerate;
	int    samples;
	int    channels;
	double max;
	double *t;
	double **a;
} wav_t, *pwav_t;

#define _wav_c_
#include <wav.h>
#undef _wav_c_

double _abs(double x) {
	if (x < 0) return -x;
	return x;
}

size_t wav_sizeof() { return sizeof(wav_t); }

pwav_t wav_destroy(pwav_t s) {
	if (s) {
		if (s->t) mem_free(s->t);
		if (s->a) {
			int i;
			for (i = 0; i < s->channels; i++) 
				if (s->a[i]) s->a[i] = mem_free(s->a[i]);
			s->a = mem_free(s->a);
		}
		free(s);
	}
	return NULL;
}

#define BKS 1024 
pwav_t wav_new(char *fname) {
	pwav_t p = NULL;
	int c, i, m, n;
	SNDFILE *sf = NULL;
	SF_INFO sfinfo;
	double  *buf = NULL, t, dt;

	memset(&sfinfo, 0, sizeof (sfinfo));
	
	if (!(sf = sf_open(fname, SFM_READ, &sfinfo))) return NULL;

	if (sfinfo.format != (SF_FORMAT_WAV | SF_FORMAT_PCM_16)) {
		err_error("Cannot open file %s since not 16bits WAV", fname);
		sf_close(sf);
		return NULL;
	}

	if (!(p = (pwav_t) mem_malloc(sizeof(wav_t)))) {
		err_warning("Cannot allocate memory for sound file data");
		sf_close(sf);
		return NULL;
	}

	if (!(p->t = (double *) mem_malloc((size_t) (sfinfo.frames + 1) * sizeof(double)))) {
		err_warning("Cannot allocate memory for soundfile time data");
		sf_close(sf);
		return wav_destroy(p);
	}
	if (!(p->a = (double **) mem_malloc(sfinfo.channels * sizeof(double *)))) {
		err_warning("Cannot allocate memory for data channels");
		sf_close(sf);
		return wav_destroy(p);
	}
		
	for (c = 0; c < sfinfo.channels; c++) {
		if (!(p->a[c] = (double *) mem_malloc((size_t) sfinfo.frames * sizeof(double)))) {
			err_warning("Cannot allocate memory for channel %d data");
			sf_close(sf);
			return wav_destroy(p);
		}
	}
	
	if (!(buf = (double *) mem_malloc((size_t) (sfinfo.channels) * BKS * sizeof(double)))) {
		err_warning("Cannot allocate memory for temporary reading buffer");
		sf_close(sf);
		return wav_destroy(p);
	}
	m = 0;
	t = 0.;
	dt = 1. / sfinfo.samplerate;

	p->max = 0;
	while ((n = sf_readf_double(sf, buf, BKS)) > 0) {
		for (i = 0; i < n; i++) {
			p->t[m] = t; 
			for (c = 0; c < sfinfo.channels; c++) {
				double A;
				A = buf[i *  sfinfo.channels + c];
				if (_abs(A) > p->max) p->max = _abs(A);
				p->a[c][m] = A;
			}
			t += dt;
			m++;
		}
	}	

	mem_free(buf);
	sf_close(sf) ;
	
	p->samples    = m;
	p->channels   = sfinfo.channels;
	p->frames     = sfinfo.frames; 
	p->samplerate = sfinfo.samplerate;

	err_log("channels   : %d", p->channels);
	err_log("frames     : %d", p->frames);
	err_log("samples    : %d", m);
	err_log("samplerate : %d", p->samplerate);
	err_log("duration   : %.2f s", (double) p->samples / (double) p->samplerate);

	return p;	
}

double wav_length(pwav_t s) {
	if (!s) return 0;
	return (double) s->samples / (double) s->samplerate;
}

int wav_samplerate(pwav_t s) {
	if (!s) return 0;
	return s->samplerate;
}

int wav_frames(pwav_t s) {
	if (!s) return 0;
	return s->frames;
}

int wav_channels(pwav_t s) {
	if (!s) return 0;
	return s->channels;
}

int wav_samples(pwav_t s) {
	if (!s) return 0;
	return s->samples;
}

double wav_time(pwav_t s, int i) {
	if (!s || !s->t) 
		return 0.;
	if (i < 0 || i >= s->samples) 
		return 0.;
	return s->t[i];
}

double wav_amplitude(pwav_t s, int chan, int i) {
	if (!s || !s->a || (chan < 0 || s->channels <= chan) || i < 0 || i >= s->samples) return 0.;
	return s->a[chan][i];
}

double wav_normalize(pwav_t s) {
	int i, c;
	double factor;
	if (!s || !s->a) return 0.;
	factor = 1. / s->max;
	for (i = 0; i < s->frames; i++) 
		for (c = 0; c < s->channels; c++) 		
			s->a[c][i] *= factor;
	return factor;
}

double wav_normalize_channel(pwav_t s, int chan, double norm) {
	int i;
	if (!s || !s->a || (chan < 0 || s->channels <= chan)) return 0.;
	for (i = 0; i < s->frames; i++) s->a[chan][i] *= norm;
	return norm;
}

double * wav_time_array(pwav_t s) {
	if (!s || !s->t) 
		return NULL;
	return s->t;
}

double * wav_amplitude_array(pwav_t s, int chan) {
	if (!s || !s->a || (s->channels <= 0 || s->channels <= chan)) return NULL;
	return s->a[chan];
}


