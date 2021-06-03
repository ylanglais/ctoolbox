#ifndef _nqc_h_
#define _nqc_h_


#ifdef __cplusplus
extern "C" {
#endif

#include <tbx/ssl.h>

#include "msg_defs.h"
#include "msg.h"

#ifndef _nqc_c_
typedef void * pnqc_t;
#endif

size_t nqc_sizeof();

pnqc_t  nqc_new(char *host, int port);
pnqc_t  nqc_ssl_new(char *host, int port, ssl_method_t method, char *certificate, 
			        char *key, long options, long verify_options, int verify_depth);

pnqc_t  nqc_destroy(pnqc_t nqc);


merr_t 	nqc_msg_send(pnqc_t nqc, pmsg_t msg);
pmsg_t  nqc_msg_recv(pnqc_t nqc);

int     nqc_put(pnqc_t nqc, char *data, size_t size);
int     nqc_put_file(pnqc_t nqc, char *filename);
int     nqc_get(pnqc_t nqc, char **data, size_t *size);
int 	nqc_get_file(pnqc_t nqc, char *filename);


#ifdef __cplusplus
}
#endif

#endif
