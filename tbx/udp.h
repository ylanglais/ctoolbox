#ifndef _udp_h_
#define _udp_h_
#if 0
#include <tbx/tstmr.h>
#include <tbx/ssl.h>
#endif

#ifndef _udp_c_ 
typedef void *pudp_t;
#endif
#if 0
enum {
	udpUNKNOWN = 0,
	udpCREATING,
	udpCONFIGURING,
	udpBINDING,
	udpCONNECTING,
	udpLISTENING
};

typedef int (*udp_cli_dialog_f)(pudp_t, int, int, void *); 
typedef int (*udp_must_stop_f)(void *);
#endif

#ifdef __cplusplus
extern "C" {
#endif

size_t udp_sizeof();
pudp_t udp_destroy(pudp_t s);
pudp_t udp_new(char *address, int port, int maxclients);

int    udp_send(pudp_t s, char *data, size_t size);
char * udp_recv(pudp_t s, size_t *size);



#if 0
int     udp_status(pudp_t s);
int     udp_port(pudp_t s);
int     udp_socket(pudp_t s);
int     udp_maxclients(pudp_t s);
int	    udp_clients_connected(pudp_t s);
#define udp_clis_connected udp_clients_connected 
void    udp_stop(pudp_t s);

char *  udp_cipher(pudp_t s);
char *  udp_certificate(pudp_t s);

/* Automatic server forking when conneced: */
int		udp_cli_connect_loop(pudp_t s, udp_cli_dialog_f udp_cli_dialog_cb, udp_must_stop_f udp_must_stop_cb, void *data);

/* Automatic server forking when connected, but w/ timout : */
int		udp_cli_connect_loop_to(pudp_t s, udp_cli_dialog_f udp_cli_dialog_cb, udp_must_stop_f udp_must_stop_cb, void *data, tstamp_t timeout);

/* To be called in parent process afer a fork(): */
int     udp_clean_connection(pudp_t s);

char *  udp_cli_connected_host(pudp_t s);
int 	udp_cli_connect_to(pudp_t s, tstamp_t timeout);
int     udp_cli_connect(pudp_t s);

int     udp_cli_disconnect(pudp_t s);

int     udp_recv_to(pudp_t s, char *data, size_t size, tstamp_t timeout);
int     udp_recv(pudp_t s, char *data, size_t size);

int     udp_send_to(pudp_t s, char *data, size_t size, tstamp_t timeout);
int     udp_send(pudp_t s, char *data, size_t size);
#endif

#ifdef __cplusplus
}
#endif

#endif
