#ifndef _ampl_h_
#define _ampl_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ampl_c_
typedef void *pampl_t;
typedef unsigned long ulong_t;
#endif

typedef enum {
	ampl_opSET = 0,
	ampl_opADD,
	ampl_opSUB,
	ampl_opMUL,
	ampl_opDIV,
	ampl_opBAD
} amplOP;

#define amplDOUBLE_ERROR ((double) -99999.12345)

#include "wav.h"

/* Creator / Destructor: */

/* Duration in ms, sample rate in n/s */
pampl_t ampl_new(double duration, ulong_t samplerate);
pampl_t ampl_destroy(pampl_t a);
pampl_t ampl_dup(pampl_t a);

int  	ampl_remove(pampl_t a, double from, double to);
pampl_t	ampl_cut(pampl_t a, double from, double to);
pampl_t ampl_copy(pampl_t a, double from, double to);
int     ampl_paste(pampl_t dst, double at, pampl_t src);
int     ampl_insert(pampl_t dst, double at, pampl_t src);
int     ampl_append(pampl_t dst, pampl_t src);
int  	ampl_blank(pampl_t a, double from, double to);
int     ampl_insert_blank(pampl_t a, double at, double duration);
int     ampl_append_blank(pampl_t a, double duration);

/* From / to wav: */
pampl_t ampl_wav_load(char *fname, int channel);
pampl_t ampl_from_wav(pwav_t wav, int channel);
int     ampl_wav_save(pampl_t a, char *fname);

/* Resampling: */
pampl_t ampl_resample(pampl_t a, ulong_t samplerate);

/* Data access: */
ulong_t ampl_samples(pampl_t a);
ulong_t ampl_samplerate(pampl_t a);
double  ampl_duration(pampl_t a);
double  ampl_tunit(pampl_t a);
double *ampl_amplitudes(pampl_t a);
double  ampl_amplitude_at_t(pampl_t a, double t);

/* Basic operations: */
int     ampl_mul(pampl_t a, double k);
int     ampl_div(pampl_t a, double k);
int     ampl_normalize(pampl_t a);

/* Amplitude arra operations: */
int     ampl_add(pampl_t a, pampl_t b, double at_a, double at_b, double duration);
int     ampl_sub(pampl_t a, pampl_t b, double at_a, double at_b, double duration);

/* sine signal generation: */
int     ampl_gen(pampl_t a, amplOP op, double amplitude, double fromtime, double totime, double freq, double phase);

/* to file or to stdio dump: */
void    ampl_dump(pampl_t a, char *filename);

#ifdef __cplusplus
}
#endif

#endif

