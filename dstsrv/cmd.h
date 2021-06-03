
#ifndef _cmd_h_
#define _cmd_h_

#include <tbx/tstmr.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef char hostident_t[110];

size_t  cmd_header_sizeof();
size_t  cmd_sizeof();
size_t  cmd_sizeof_n(int numcmds);

void *cmd_init(int key, size_t size, char *filename);
void *cmd_force_init(int key, size_t size, char *filename);
void *cmd_exit(void *p);

pid_t cmd_pid_get(void *p);
void  cmd_pid_update(void *p);

int   cmd_number(void *p);
int   cmd_completed(void *p);
int   cmd_remaining(void *p);

int   cmd_spec_read(void *p, char *filename);
void  cmd_spec_write(void *p, char *filename);
void  cmd_report_write(void *p, char *filename);
int   cmd_error(void *p, char *filename);

void  cmd_add(void *p, long duration, char *cmd);
char *cmd_next(void *p, int *token, int maxrun);

void  cmd_start(void *p, int token, hostident_t host, tstamp_t at);
void  cmd_resume(void *p, int token);
void  cmd_respool(void *p, int token);
void  cmd_end(void *p, int token, int r, tstamp_t ended, long elapsed);
void  cmd_client_died(void *p, char *hostident);
void  cmd_dump_all(void *p);
int   cmd_running_check(void *p, tstamp_t timeout);

#ifdef __cplusplus
}
#endif

#endif
