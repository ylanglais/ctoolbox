#ifndef _rexp_h_
#define _rexp_h_

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct _rexp_t *p_rexp_t;
typedef struct _rexp_t *prexp_t;


/* Possible flags: */
#define rexp_EXTENDED 			1
#define rexp_IGNORECASE 		(rexp_EXTENDED << 1)
#define rexp_NL_DO_NOT_MATCH	(rexp_IGNORECASE << 1)

size_t   rexp_sizeof(); 
p_rexp_t rexp_new(char *buffer, char *reg_exp, int flags);
p_rexp_t rexp_free(p_rexp_t prexp);
p_rexp_t rexp_destroy(p_rexp_t prexp);

char *   rexp_buffer_get(p_rexp_t prexp);
p_rexp_t rexp_buffer_set(p_rexp_t prexp, char *buffer);

int      rexp_count_match(p_rexp_t prexp);

int      rexp_find(p_rexp_t prexp);
char *   rexp_match_get(p_rexp_t prexp);
int      rexp_match_index_get(p_rexp_t prexp);

size_t   rexp_nsubs(p_rexp_t prexp);
char *   rexp_sub_get(p_rexp_t prexp, int index);

int 	rexp_replace_first(p_rexp_t prexp, char *string);
int 	rexp_replace_next(p_rexp_t prexp, char *string);
int 	rexp_replace_next_with_func(p_rexp_t prexp, char * (*)(p_rexp_t));
int 	rexp_replace_all(p_rexp_t prexp, char *string);
int 	rexp_replace_all_with_func(p_rexp_t prexp, char * (*)(p_rexp_t));

#if defined(__DEBUG__)
#if !defined(_test_rexp_c_)
int		rexp_test(void);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
