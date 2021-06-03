#ifndef _msg_h_
#define _msg_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef  _msg_defs_h_ 
#include "msg_defs.h"
#endif

#ifndef _msg_c_
typedef void *	pmsg_hdr_t; 
typedef void *	pmsg_ok_t;
typedef void *	pmsg_ko_t;
typedef void *  pmsg_put_t;
typedef void *  pmsg_get_t;
typedef void *  pmsg_t;
#endif

/* message creators & destructor: */
pmsg_t          msg_new(pmsg_hdr_t hdr);
pmsg_t          msg_hdr_new(msg_command_t cmd, unsigned int seq, unsigned int size);
pmsg_t 			msg_ok_new(unsigned int seq);
pmsg_t          msg_ko_new(unsigned int seq, merr_t err, int action, char *message);
pmsg_t          msg_put_new(unsigned int seq, char *data, size_t size, unsigned long crc);
pmsg_t 			msg_get_new(unsigned int seq);
pmsg_t 			msg_destroy(pmsg_t msg);

void			msg_dump(pmsg_t msg);

int             msg_seq_incr(pmsg_t msg);
int				msg_check(pmsg_t msg);

char *          msg_err_string(merr_t err);
char *          msg_cmd_string(msg_command_t cmd);
msg_command_t 	msg_cmd(pmsg_t  msg);
size_t 			msg_size(pmsg_t msg);
unsigned int    msg_seq(pmsg_t  msg);
char *			msg_payload(pmsg_t msg);

size_t			msg_hdr_sizeof();

size_t			msg_hdr_size(pmsg_t msg);
size_t			msg_load_size(pmsg_t msg);
size_t			msg_full_size(pmsg_t msg);

char *          msg_put_data(pmsg_t msg);
size_t          msg_put_size(pmsg_t msg);
unsigned long   msg_put_crc(pmsg_t msg);

merr_t			msg_ko_err(pmsg_t msg);
char *			msg_ko_err_string(pmsg_t msg);
msg_ko_action_t	msg_ko_action(pmsg_t msg);
char *			msg_ko_message(pmsg_t msg);

#ifdef __cplusplus
}
#endif

#endif
