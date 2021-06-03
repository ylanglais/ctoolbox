#ifndef _tuples_h_
#define _tuples_h_

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long   tuples_card(int n, int k);
int *                tuples_i(int n, int k, unsigned long long i);

#ifdef __cplusplus
}
#endif

#endif
