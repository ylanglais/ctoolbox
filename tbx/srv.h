#ifndef _srv_h_
#define _srv_h_

#include <tbx/tstmr.h>
#include <tbx/ssl.h>

#ifndef _srv_c_ 
typedef void *psrv_t;
#endif

enum {
	srvUNKNOWN = 0,
	srvCREATING,
	srvCONFIGURING,
	srvBINDING,
	srvCONNECTING,
	srvLISTENING
};

typedef int (*srv_cli_dialog_f)(psrv_t, int, int, void *); 
typedef int (*srv_must_stop_f)(void *);

#ifdef __cplusplus
extern "C" {
#endif

size_t srv_sizeof();

psrv_t srv_destroy(psrv_t s);

psrv_t srv_new(char *address, int port, int maxclients);
psrv_t srv_ssl_new(char *address, int port, int maxclients, 
				   ssl_method_t method, char *certificate, char *key, 
				   long options, long verif_options, int verify_depth);

int     srv_status(psrv_t s);
int     srv_port(psrv_t s);
int     srv_socket(psrv_t s);
int     srv_maxclients(psrv_t s);
int	    srv_clients_connected(psrv_t s);
#define srv_clis_connected srv_clients_connected 
void    srv_stop(psrv_t s);

char *  srv_cipher(psrv_t s);
char *  srv_certificate(psrv_t s);

/* Automatic server forking when conneced: */
int		srv_cli_connect_loop(psrv_t s, srv_cli_dialog_f srv_cli_dialog_cb, srv_must_stop_f srv_must_stop_cb, void *data);

/* Automatic server forking when connected, but w/ timout : */
int		srv_cli_connect_loop_to(psrv_t s, srv_cli_dialog_f srv_cli_dialog_cb, srv_must_stop_f srv_must_stop_cb, void *data, tstamp_t timeout);

/* To be called in parent process afer a fork(): */
int     srv_clean_connection(psrv_t s);

char *  srv_cli_connected_host(psrv_t s);
int 	srv_cli_connect_to(psrv_t s, tstamp_t timeout);
int     srv_cli_connect(psrv_t s);

int     srv_cli_disconnect(psrv_t s);

int     srv_recv_to(psrv_t s, char *data, size_t size, tstamp_t timeout);
int     srv_recv(psrv_t s, char *data, size_t size);

int     srv_send_to(psrv_t s, char *data, size_t size, tstamp_t timeout);
int     srv_send(psrv_t s, char *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
