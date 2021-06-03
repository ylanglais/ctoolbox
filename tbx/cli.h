#ifndef _cli_h_
#define _cli_h_

#ifndef _cli_c_
#include <netdb.h>
typedef void *pcli_t;
#endif

#include <tbx/tstmr.h>
#include <tbx/ssl.h>

#ifdef __cplusplus
extern "C" {
#endif

void cli_dump(pcli_t c);

size_t cli_sizeof();

pcli_t cli_destroy(pcli_t c);
pcli_t cli_new(char *host, int port);
pcli_t cli_ssl_new(char *host, int port, 
		ssl_method_t method, char *certificate, char *key, 
		long options, long verif_options, int verify_depth);

pcli_t cli_reconnect(pcli_t c); 

char  *cli_hostname(pcli_t c);
int    cli_socket(pcli_t c);
int    cli_port(pcli_t c);

char * cli_cipher(pcli_t c);
char * cli_certificate(pcli_t c);

int    cli_recv_opts_set(pcli_t c, int recv_opts);
int    cli_send_opts_set(pcli_t c, int recv_opts);

int    cli_send_to(pcli_t c, char *data, size_t size, tstamp_t timeout);
int    cli_send(pcli_t c, char *data, size_t size);

int    cli_recv_to(pcli_t c, char *data, size_t size, tstamp_t timeout);
int    cli_recv(pcli_t c, char *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
