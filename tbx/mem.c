#include "mem.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	04/08/2016  1.2  add changelog & tbx tags
*/   

char mem_MODULE[]  = "mem";
char mem_PURPOSE[] = "memory function helpers";
char mem_VERSION[] = "1.2";
char mem_DATEVER[] = "04/08/2016";

void *
mem_zmalloc(size_t s) {
    void *p;
    if (!(p = malloc(s))) return NULL;
    memset((char *) p, 0, s);
    return p;
}    

void *
mem_free(void *p) {
    if (p) free(p);
    return NULL;
}    
