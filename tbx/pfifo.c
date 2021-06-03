

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
    08/12/2017  0.9 	Initial version
*/   

#include <stdlib.h>
#ifdef __reentrant__
#include <pthread.h>
#define pfifo_lock(x)   pthread_mutex_lock((x)->mp)
#define pfifo_unlock(x) pthread_mutex_unlock((x)->mp)
#else
#define pfifo_lock(x)
#define pfifo_unlock(x)
#endif
#include "pal.h"



