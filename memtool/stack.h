#ifndef _stack_h_
#define _stack_h_

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(SUN) && !defined(LINUX) 
#error : stack only works on Solaris and Linux
#endif

void stack_init(void *from, size_t size);
void *stack_curr_get(); 
int stack_print(void *ptr);
void stack_dump(int n, void *ptr);
void stack_live_dump();
int stack_trace();

#ifdef __cplusplus
}
#endif

#endif
