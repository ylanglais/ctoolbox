#ifndef _pdu_h_
#define _pdu_h_

#include <smpp_def.h>

typedef struct {
	int command_len;
	int command_id;
	int command_status;
	int seq_num;	
} pdu_t, *ppdu_t, pduhdr_t, *ppduhdr_t; 


#ifdef __cplusplus
extern "C" {
#endif
size_t pdu_len(ppdu_t p);
int    pdu_cmd(ppdu_t p);
int    pdu_seq(ppdu_t p);
int    pdu_status(ppdu_t p);

void   pdu_hexdump(ppdu_t p);

#ifdef __cplusplus
}
#endif

#endif
