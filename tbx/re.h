#ifndef _re_h_
#define _re_h_

#ifdef __cplusplus
extern "C" {
#endif 

/* Flags: */
#define reIGNORECASE 0x0001
#define reNEWLINE    0x0002
#define reNOSUB      0x0004
#define reEXTENDED	 0x0008
#define reDOTALL     0x0010
#define reUTF8		 0x0800

#define reMULTILINE  reNEWLINE

#define RE_MAX_SUB 	 30

typedef struct {
	int	   so;	
	int    eo;	
} resub_t, *presub_t;

typedef struct _rematch_t {
	int    nsubs;
	presub_t subs; 
} rematch_t, *prematch_t;	

#if defined(_re_c_) || defined(_re_posix_c_) || defined(_re_pcre_c_) || defined(_re_pcre2_c_) 
typedef struct _re_t {
	void *      r;
	char *      buffer;
	char *      rexp;
	char        flags;
	int         (*re_mod_new)(struct _re_t *);
	int         (*re_mod_compile)(struct _re_t *, int);
	void *      (*re_mod_free)(struct _re_t *);
	struct _rematch_t*  (*re_mod_match)(struct _re_t *, char *);
} re_t, *pre_t;

#else 
typedef void *pre_t;
#endif

typedef int    (*re_match_processor_f)  (pre_t, void *, prematch_t);

/* Create/dispose rematch_t matching offset array struct: */
prematch_t rematch_new(int n);
prematch_t rematch_destroy(prematch_t m);
size_t     re_sizeof();

/* Create and dispose regular expresssion structure: */
pre_t      re_new(char *buffer, char *rexp, int flags);
pre_t      re_destroy(pre_t r);

/* Get buffer: */
char *     re_buffer(pre_t r);
char * 	   re_substr(pre_t r, int from, int to);

/* Convert regmatch structure into an array of 10 strings: */
char **    remstrs_new(pre_t r, prematch_t m);
char **    remstrs_destroy(char **s);

/* Find matching strings: */
prematch_t re_find(pre_t r, char *p);
prematch_t re_find_first(pre_t r);
prematch_t re_find_next(pre_t r, char *ptr);
int        re_find_foreach(pre_t r, void *user_data, re_match_processor_f process);
int        re_match_count(pre_t r);

/* Replace matching strings: */
char *     re_replace_first(pre_t r, char *tmpl);
char *     re_replace_next(pre_t r, char *from, char *tmpl);
int        re_replace_all(pre_t r, char *tmpl);

#ifdef __cplusplus
};
#endif
#endif
