#ifndef _coverage_h_
#define _coverage_h_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__COVERAGE__) && defined(HAS__FUNC__)
#define COVIN  coverage(__FILE__, __LINE__, __func__, 1);
#define COVOUT coverage(__FILE__, __LINE__, __func__, 0); 
#define COVCP(n) coverage(__FILE__, __LINE__, __func__, n);
#define COV_RETURN(...) {COVOUT return  __VA_ARGS__  ;}
#else
#define COVIN 
#define COVOUT
#define COV_RETURN(...) return __VA_ARGS__ ;
#if defined(__COVCP__) && defined(HAS__FUNC__)
#define COVCP(n) dbg_coverage(__FILE__, __LINE__, __func__, n);
#else
#define COVCP(n)
#endif
#endif

void coverage(char *file, int line, char *function, int flag);

#include <sys/time.h>
struct timeval cov_time_zero();
struct timeval cov_time_get();
struct timeval cov_time_sub(struct timeval tv0, struct timeval tv1);
struct timeval cov_time_add(struct timeval tv0, struct timeval tv1);
char *cov_time_fmt(struct timeval tv);

#ifdef __cplusplus
}
#endif

#endif
