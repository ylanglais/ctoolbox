#include <stdlib.h>
#include <string.h>

#include <tbx/err.h>
#include <tbx/option.h>
#include <tbx/futl.h>
#include <tbx/srv.h>
#include <tbx/ssl.h>
#include <tbx/crc_32.h>
#include <tbx/tstmr.h>

#include "msg.h"
#include "nq.h"
#include "qe.h"

#define nqsHOST_LEN		100
#define nqsPATH_LEN		500
#define nqsCERT_LEN		500
#define nqsCKEY_LEN		500

typedef struct {
	psrv_t  srv;
	int		debug;
	int 	maxclient;
	char 	host[nqsHOST_LEN + 1];
	int		port;
	char	path[nqsPATH_LEN + 1];
	//ptmr_t	timer;
	int		ssl;
	int		ssl_chk;
	int		ssl_depth;
	int		ccert;
	char	cert[nqsCERT_LEN + 1];
	char	ckey[nqsCKEY_LEN + 1];
	pnq_t    nq;
} nqs_t, *pnqs_t;

nqs_t nqs_init() {
	nqs_t nqs;
	nqs.srv       = NULL;
	nqs.debug     =    0;
	nqs.port      =   -1;
	nqs.ssl	      =    0;
	nqs.ssl_chk   =    0;
	nqs.ssl_depth =    0;
	nqs.ccert     =    0;
	memset(nqs.host, 0, nqsHOST_LEN + 1);
	memset(nqs.path, 0, nqsPATH_LEN + 1);
	memset(nqs.cert, 0, nqsPATH_LEN + 1);
	memset(nqs.ckey, 0, nqsPATH_LEN + 1);
	return nqs;
}
void nqs_dump(nqs_t nqs) {
	err_debug("maxclient = %d", nqs.maxclient);
	err_debug("host      = %s", nqs.host     );
	err_debug("port      = %d", nqs.port     );
	err_debug("debug     = %d", nqs.debug    );
	err_debug("path      = %s", nqs.path     );
	err_debug("ssl       = %d", nqs.ssl      );
	err_debug("ssl_chk   = %d", nqs.ssl_chk  );
	err_debug("ssl_depth = %d", nqs.ssl_depth);
	err_debug("cert      = %s", nqs.cert     );
	err_debug("ckey      = %s", nqs.ckey     );
	err_debug("ccert     = %d", nqs.ccert    );
}

int nqs_check(nqs_t nqs) {
	if (nqs.port < 0) {
		err_error("invalid port number (%d)", nqs.port);
		return 1;
	}
	if (!nqs.path[0]) {
		err_error("no path nqiven");
		return 2;
	}
	if (!futl_is_dir(nqs.path)) {
		err_error("path is not reachable or is not a directory");
		return 3;
	}
	if (!futl_is_rwx(nqs.path)) {
		err_error("bad permissions");
		return 4;
	}

	if (nqs.ssl) {
		if (!nqs.cert[0]) {
			err_error("ssl renquired but no certificate file given");
			return 5;
		} 
		if (!futl_is_r(nqs.cert)) {
			err_error("ssl renquired but certificate file not readable");
			return 6;
		} 
		if (!nqs.ckey[0]) {
			err_error("ssl renquired but no key file given");
			return 7;
		} 
		if (!futl_is_r(nqs.ckey)) {
			err_error("ssl renquired key file not readable");
			return 8;
		} 
	}
	if (nqs.ccert && !nqs.ssl) {
		err_warning("client certificate ignored since it is set as renquired but ssl is turned off");
		nqs.ccert = 0;
	}
	return 0;
}

merr_t 
nqs_msg_send(pnqs_t nqs, pmsg_t msg) {
	size_t sz, szr;

	if (!nqs || !nqs->srv) {
		err_error(msg_err_string(merr_BAD_QUEUE_SERVER));
		return merr_BAD_QUEUE_SERVER;
	}
	if (!msg) {
		err_error(msg_err_string(merr_NULL_MESSAGE));
		return merr_NULL_MESSAGE;
	}
	sz = msg_full_size(msg);	
	if ((szr = srv_send(nqs->srv, (char *) msg, sz)) != sz) { 
		err_error("message only partially sent (%lu instead of %lu)", szr, sz);
		return merr_PARTIAL_MESSAGE;
	} 
	err_debug("message sent (%lu bytes)", sz);
	return merr_NONE; 
}

pmsg_t
nqs_msg_recv(pnqs_t nqs) {
	pmsg_hdr_t hdr;
	pmsg_t     msg;
	size_t	   szr;
	size_t     sz = msg_hdr_sizeof();

	if (!nqs || !nqs->srv) {
		err_error("null server"); 
		return NULL;
	} 

	hdr = msg_hdr_new(msg_BAD, 0, 0);

	if ((szr = srv_recv(nqs->srv, (char *) hdr, sz)) != sz) { 
		err_error("partial content (%lu bytes received instead of %lu)", szr, sz);
		return NULL;
	}  

	if (msg_check(hdr)) {
		err_error("bad message");
		return NULL;
	}
	
	err_debug("received header from %s message of size %lu", msg_cmd_string(msg_cmd(hdr)), msg_load_size(hdr));

	if (!(sz = msg_load_size(hdr))) return hdr;

	err_debug("receive message payload:");

	if (!(msg = msg_new(hdr))) {
		err_error("cannot create message from header");
		msg_destroy(hdr);
		return NULL;
	}

err_debug("msg = %p, payload = %p (delta = %d vs msg_hdr_sizeof %lu)", msg, msg_payload(msg), 
		 (char *) msg_payload(msg) - (char *) msg, msg_hdr_sizeof());
err_debug("receive payload");

	if ((szr = srv_recv(nqs->srv, msg_payload(msg), sz)) != sz) {
		err_error("partial content (%lu bytes received instead of %lu)", szr, sz);
	} 
	err_debug("received %lu", szr);
	msg_destroy(hdr);
	return msg;
}

merr_t
nqs_srv_put_request(pnqs_t nqs, pmsg_t msg) {
	pmsg_t  rsp  = NULL;
	merr_t  err  = merr_NONE;
	nqerr_t qerr = nqerr_NONE;
	msg_command_t cmd; 

	err_debug("in nqs_srv_put_request");

	if (!nqs) {
		err = merr_BAD_QUEUE_SERVER;
		err_error(msg_err_string(err));
		goto cleanup;
	}
	if (!msg)  {
		err = merr_NULL_MESSAGE;
		err_error(msg_err_string(err));
		goto cleanup;
	}

	int seq = msg_seq(msg);

	while (1) {
		if ((cmd = msg_cmd(msg)) != msg_PUT) {
			err_error("incorrect message (%s instead of %s)", msg_cmd_string(cmd), msg_cmd_string(msg_PUT));
			err = merr_INVALID_PROTOCOL;
			nqs_msg_send(nqs, rsp = msg_ko_new(seq + 1, err, act_BYE, msg_err_string(err)));
			goto cleanup;
		}

		err_debug("chck queue server");
		if (!nqs->nq) {
			err = merr_BAD_QUEUE;
			err_error(msg_err_string(err));
			nqs_msg_send(nqs, rsp = msg_ko_new(seq + 1, err, act_BYE, msg_err_string(err)));
			goto cleanup;
		}

		err_debug("compute crc");
		unsigned long crc = crc_32((unsigned char *) msg_put_data(msg), msg_put_size(msg));
		err_debug("crc is %lu", crc);

		if (crc == msg_put_crc(msg)) {
			err_debug("queue put");

			if ((qerr = nq_put(nqs->nq, tstamp_get(), msg_put_data(msg), msg_put_size(msg), msg_put_crc(msg))) == nqerr_NONE) {
				err_debug("message put");
				nqs_msg_send(nqs, rsp = msg_ok_new(seq + 1));
				err = merr_NONE;
				goto cleanup;
			}
		} else err = merr_INVALID_CHECKSUM;

		if (qerr != nqerr_NONE) {
			err_warning("queue error: %s", nq_err_string(qerr));
			err = merr_QUEUE_ERROR;
		} else 
			err_warning("%s", msg_err_string(err));

		if (seq > 5) {
			err = merr_MAX_RETRY;
			err_error(msg_err_string(err));
			nqs_msg_send(nqs, rsp = msg_ko_new(seq + 1, err, act_BYE, msg_err_string(err)));
			goto cleanup;
		}
		
		nqs_msg_send(nqs, rsp = msg_ko_new(seq + 1, err, act_RETRY, msg_err_string(err)));
			
		if (msg) msg = msg_destroy(msg);
		if (rsp) rsp = msg_destroy(rsp);
		msg = nqs_msg_recv(nqs);
		seq = msg_seq(msg);
	}

	cleanup:
	if (rsp) rsp = msg_destroy(rsp);
	if (msg) msg = msg_destroy(msg);
	return err;
}

int
nqs_srv_get_request(pnqs_t nqs, pmsg_t msg) {
	pmsg_t put = NULL, rsp = NULL;
	char *data; 
	size_t size;
	unsigned long crc;
	msg_command_t cmd; 
	nqerr_t qerr = nqerr_NONE;
	merr_t  err  = merr_NONE;

	/* TODO: include the whole protocole within this function */
	if (!nqs) {
		err_error("Bad nqueue server");
		err = merr_BAD_QUEUE_SERVER;
		goto cleanup;
	}
	if (!msg)  {
		err_error("Null message");
		err =  merr_NULL_MESSAGE;
		goto cleanup;
	}
		
	if ((cmd = msg_cmd(msg)) != msg_GET) {
		err_error("incorrect message (%s instead of %s)", msg_cmd_string(cmd), msg_cmd_string(msg_GET));
		err = merr_INCORRECT_MESSAGE;
		goto cleanup;
	}

	err_debug("get message from queue");

	if ((qerr = nq_get(nqs->nq, &data, &size, &crc)) != nqerr_NONE) {
		err_debug("qerr %d (%s)", nq_err_string(qerr));
		if (qerr == nqerr_EMPTY) {
			nqs_msg_send(nqs, rsp = msg_ko_new(msg_seq(msg) + 1, merr_EMPTY, act_BYE, "empty nqueue"));
			err_log("empty nqueue");
			goto cleanup;
		} else {
			nqs_msg_send(nqs, rsp = msg_ko_new(msg_seq(msg) + 1, merr_QUEUE_ERROR, act_BYE, nq_err_string(qerr)));
			err_error("nqueue error (%s)", nq_err_string(qerr));
			err = merr_QUEUE_ERROR;
			goto cleanup;
		}
	}	

	err_debug("create put message with data at %p of size %lu and crc %lu", data, size, crc);	

	/* create put message: */
	put = msg_put_new(msg_seq(msg), data, size, crc);

	while (1) {
		msg_seq_incr(rsp);
		err_debug("send a put message for data at %p of size %lu with crc %lu", data, size, crc);
		if ((err = nqs_msg_send(nqs, rsp)) != merr_NONE) {
			err_error("problem sending requested put message to client (%s)", msg_err_string(err));
			goto cleanup;
		}

		if (!(msg = nqs_msg_recv(nqs))) {
			err_log("null message");
			err = merr_NULL_MESSAGE;
			goto cleanup;
		}

		msg_seq_incr(put);

		if (msg_cmd(msg) == msg_OK) {
			err_log("message has been received by client");
			err = merr_NONE;
			goto cleanup;
		} 
		if (msg_cmd(msg) == msg_KO) {
			if (msg_ko_action(msg) == act_BYE || msg_seq(msg) > 6) {
				err_log("cannot send data => requeue w/ new stamp");
				nq_requeue(nqs->nq, tstamp_get(), data, size, crc);
				nqs_msg_send(nqs, rsp = msg_ko_new(msg_seq(msg) + 1, merr_REQUEUE, act_BYE, "requeing"));
				err = merr_MAX_RETRY;
				goto cleanup;
			} else { 
				err_log("retry sending data");
				nqs_msg_send(nqs, rsp = msg_put_new(msg_seq(msg) + 1, data, size, crc));
				msg_destroy(msg);
				msg_destroy(rsp);
			}
		} else {
			char b[500];
			nq_requeue(nqs->nq, tstamp_get(), data, size, crc);
			err_error("requeuing after Incorrect message (%s)", msg_cmd_string(msg_cmd(msg)));
			snprintf(b, 499, "requeuing after incorrect message (%s)", msg_cmd_string(msg_cmd(msg)));
			nqs_msg_send(nqs, rsp = msg_ko_new(msg_seq(msg) + 1, merr_REQUEUE, act_BYE, b));
			err = merr_INVALID_PROTOCOL;
		}	
	}	

	cleanup:
	if (msg) msg_destroy(msg);
	if (put) msg_destroy(put);
	if (rsp) msg_destroy(rsp);

	return err;
}

int
nqs_srv_cli_dialog(psrv_t srv, int srvnum, int cid, void *nqs) {
	pmsg_t msg = NULL;
	pnqs_t s = (pnqs_t) nqs;

	err_debug("waiting for message");
	while ((msg = nqs_msg_recv(s))) {
		err_log("recieved %s message from %s", msg_cmd_string(msg_cmd(msg)), srv_cli_connected_host(s->srv));
		switch (msg_cmd(msg)) {
			pmsg_t rsp;
			case msg_BYE:
				srv_destroy(s->srv);
				exit(err_NONE);	
			case msg_PUT:
				nqs_srv_put_request(s, msg);
				srv_destroy(s->srv);
				exit(err_NONE);	
			case msg_GET:
				nqs_srv_get_request(s, msg);
				srv_destroy(s->srv);
				exit(err_NONE);	

			case msg_LCK:
				srv_destoy(s->srv);
				


			default:
				nqs_msg_send(s, rsp = msg_ko_new(msg_seq(msg) + 1, merr_UNKNOWN_COMMAND, act_BYE, msg_err_string(merr_UNKNOWN_COMMAND)));
				msg_destroy(rsp);
				msg_destroy(msg);
				srv_destroy(s->srv);
				exit(merr_UNKNOWN_COMMAND);
		}
	}
	err_error("null msg returned");
	return 0;
}

int main(int n, char *a[])  {
	int i;
	nqs_t nqs;
	popt_t opt;
	char *conffile;

	//err_init(NULL, err_WARNING);
	err_init(NULL, err_DEBUG);

	nqs = nqs_init();

	/* Parse command line : */
	for (i = 1; i < n; i++) {
		if (!strcmp(a[i], "-c")) {
			conffile = a[++i];			
/*
		} else if (!strcmp(a[i], "-s")) {
		
			specfile   = a[++i];
		} else if (!strcmp(a[i], "-e")) {
			strcpy(nqs.errfile, a[++i]);
		} else if (!strcmp(a[i], "-n")) {
			strcpy(nqs.newspec, a[++i]);
		} else if (!strcmp(a[i], "-r")) {
			strcpy(nqs.report, a[++i]);
		} else if (!strcmp(a[i], "-m")) {
			cleanshm = 1;
*/
		} else {
			err_error("unknown arguement %s", a[i]);
		}
	}
	
	if (!conffile) {
		err_error("conffile be specified on command line");
		return 1;
	}
	
	if (!(opt = opt_new("[ \t]*([A-z0-9_]+)[ \t]*=[ \t]*(.*)[ \t]*\n"))) {
		err_error("cannot create option");	
		return 2;
	}

	opt_entry_add(opt, "maxclient",            "%d", sizeof(int), &nqs.maxclient, NULL);
	opt_entry_add(opt, "debug",                "%d", sizeof(int), &nqs.debug,     NULL);
	opt_entry_add(opt, "bind",                 "%s", nqsHOST_LEN, &nqs.host,      NULL);
	opt_entry_add(opt, "port",                 "%d", sizeof(int), &nqs.port,      NULL);
	opt_entry_add(opt, "path",                 "%s", nqsPATH_LEN, &nqs.path,      NULL);
	opt_entry_add(opt, "ssl",                  "%d", sizeof(int), &nqs.ssl,       NULL);
	opt_entry_add(opt, "ssl_check",            "%d", sizeof(int), &nqs.ssl_chk,   NULL);
	opt_entry_add(opt, "ssl_depth",            "%d", sizeof(int), &nqs.ssl_depth, NULL);
	opt_entry_add(opt, "renquire_client_cert", "%d", sizeof(int), &nqs.ccert,     NULL);
	opt_entry_add(opt, "certificate", 		   "%s", nqsCERT_LEN, &nqs.cert,      NULL);
	opt_entry_add(opt, "key", 		  	       "%s", nqsCKEY_LEN, &nqs.ckey,      NULL);

	err_debug("parse config file %s", conffile);
	
	opt_parse(opt, conffile, &nqs);
	opt_destroy(opt);

	if (!(nqs.nq = nq_new(nqs.path))) exit(merr_BAD_QUEUE);

	/* Prepare loging: */
#if 0
	char buf[1000];	
	sprintf(buf, "%s/nqs.log", nqs.path);
#endif
	char *buf = NULL;
	if      (nqs.debug == 1) err_init(buf, err_DEBUG); 
	else if (nqs.debug == 2) err_init(buf, err_TRACE); 
	else if (nqs.debug == 3) err_init(buf, err_COVERAGE); 
	else 		    		 err_init(buf, err_WARNING);

	err_message("starting %s", a[0]);

	if (nqs.ssl) {
		if (nqs.ssl == 1)  nqs.ssl_chk  = SSL_VERIFY_NONE;
		else               nqs.ssl_chk  = SSL_VERIFY_PEER;
		if (nqs.ccert)     nqs.ssl_chk |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;

		err_message("create ssl server");
		nqs.srv = srv_ssl_new(nqs.host, nqs.port, nqs.maxclient, ssl_TLS, nqs.cert, nqs.ckey, 0, nqs.ssl_chk, nqs.ssl_depth);
	} else {
		err_message("create server");
		nqs.srv = srv_new(nqs.host, nqs.port, nqs.maxclient);
	}
	if (!nqs.srv) {
		err_error("cannot start nqueue, exit");
		exit(4);
	}

	srv_cli_connect_loop(nqs.srv, nqs_srv_cli_dialog, NULL, (void *) &nqs);

	srv_destroy(nqs.srv);
	return 0;
}
