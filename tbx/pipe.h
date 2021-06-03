#ifndef _pipe_h_
#define _pipe_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "pipe.h"

#ifndef _pipe_c_
typedef void *ppipe_t;
#endif

typedef enum {
	pipeUNKNOWN,
	pipeCANNOT_CREATE,
	pipeCANNOT_OPEN,
	pipeCANNOT_RECEIVE,
	pipeCANNOT_SEND,
	pipeREADY_TO_RECEIVE,
	pipeREADY_TO_SEND,
	pipeCLOSED
} pipe_status_t;

/* Listener side: */
ppipe_t pipe_create(char *file);
ppipe_t pipe_destroy(ppipe_t pi);
char *  pipe_read(ppipe_t pi); /* Must free string after use */

/* Talker side */
ppipe_t pipe_open(char *file);
ppipe_t pipe_close(ppipe_t pipe);
int     pipe_write(ppipe_t pi, char *data);

#ifdef __cplusplus
}
#endif

#endif
