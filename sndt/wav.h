#ifndef _wav_h_
#define _wav_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _wav_c_
typedef void *pwav_t;
#define wav_t wav_time
#define wav_a wav_amplitude
#define wav_t_arr wav_time_array
#define wav_a_arr wav_amplitude_array
#endif

size_t   wav_sizeof();
pwav_t   wav_new(char *fname);
pwav_t   wav_destroy(pwav_t s);
int      wav_save(pwav_t w, char *fname);
int      wav_channels(pwav_t s);
double 	 wav_max(pwav_t s);
double   wav_length(pwav_t s);
int      wav_samplerate(pwav_t s);
int      wav_samples(pwav_t s);
int      wav_frames(pwav_t s);
double   wav_time(pwav_t s, int i);
double   wav_amplitude(pwav_t s, int chan, int i);
double   wav_normalize(pwav_t s);
double 	 wav_normalize_channel(pwav_t s, int chan, double norm);

double * wav_time_array(pwav_t s);
double * wav_amplitude_array(pwav_t s, int chan);

#ifdef __cplusplus
}
#endif

#endif

