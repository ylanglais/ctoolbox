#include <tbx/err.h>
#include <tbx/cli.h>
#include <tbx/crc_32.h>
#include <tbx/futl.h>

#include "msg.h"

typedef struct {
	pcli_t	cli;
	int 	debug;
} nqc_t, *pnqc_t;

#define _nqc_c_
#include "nqc.h"
#undef _nqc_c_

size_t nqc_sizeof() { return sizeof(nqc_t); }

pnqc_t 
nqc_new(char *host, int port) {
	pnqc_t nqc;
	
	if (!(nqc = (pnqc_t) malloc(nqc_sizeof()))) {
		err_error("No memory");
		return NULL;		
	}	

	if (!(nqc->cli = cli_new(host, port))) {
		err_error("cannot connect to %s:%d", host, port);	
		return nqc_destroy(nqc);
	}
	return nqc;
}

pnqc_t nqc_ssl_new(char *host, int port, ssl_method_t method, char *certificate, char *key, long options, long verify_options, int verify_depth) {
	pnqc_t nqc;
	
	if (!(nqc = (pnqc_t) malloc(nqc_sizeof()))) {
		err_error("No memory");
		return NULL;		
	}	

	if (!(nqc->cli = cli_ssl_new(host, port, method, certificate, key, options, verify_options, verify_depth))) {
		err_error("cannot connect to %s:%d", host, port);	
		return nqc_destroy(nqc);
	}
	return NULL;
}

pnqc_t
nqc_destroy(pnqc_t nqc) {
	if (nqc) {
		if (nqc->cli) nqc->cli = cli_destroy(nqc->cli);
		free(nqc);
	}
	return NULL;
}

merr_t
nqc_msg_send(pnqc_t nqc, pmsg_t msg) {
	size_t sz, szr;

	if (!nqc || !nqc->cli) {
		err_error(msg_err_string(merr_BAD_QUEUE_CLIENT));
		return merr_BAD_QUEUE_CLIENT;
	}
	if (!msg) {
		err_error(msg_err_string(merr_NULL_MESSAGE));
		return merr_NULL_MESSAGE;
	}
	sz = msg_full_size(msg);
	if ((szr = cli_send(nqc->cli, (char *) msg, sz)) != sz) { 
		err_error("message only partially sent (%lu instead of %lu)", szr, sz);
		return merr_PARTIAL_MESSAGE;
	} 
	err_debug("message sent (%lu bytes)", sz);
	return merr_NONE; 
}

pmsg_t
nqc_msg_recv(pnqc_t nqc) {
	pmsg_hdr_t hdr;
	pmsg_t     msg;
	size_t     szr;
	size_t sz = msg_hdr_sizeof();

	if (!nqc || !nqc->cli) {
		err_error("null server"); 
		return NULL;
	} 

	hdr = msg_hdr_new(msg_BAD, 0, 0);

	if ((szr = cli_recv(nqc->cli, (char *) hdr, sz)) != sz) { 
		err_error("partial content (%lu instead of %lu)", szr, sz);
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

	if ((szr = cli_recv(nqc->cli, msg_payload(msg), sz)) != sz) {
		err_error("partial content (%lu bytes received instead of %lu)", szr, sz);
	} 
	err_debug("received %lu", szr);
	msg_destroy(hdr);
	return msg;
}

int
nqc_put(pnqc_t nqc, char *data, size_t size) {
	unsigned long crc;
	pmsg_t msg, rsp;
	int seq = 1;
	merr_t err;


	crc = crc_32((unsigned char *) data, size);
	err_debug("put message at %x of size %lu w/ crc %lu", data, size, crc);
	
	if (!(msg = msg_put_new(seq, data, size, crc))) {
		err_log("cannot create message");
		return merr_NO_MEMORY; 
	}

	while (1) {
		if ((err = nqc_msg_send(nqc, msg)) != merr_NONE) {
			err_log("cannot send message (%s)", msg_err_string(err)); 
			goto cleanup;
		} 
		err_debug("sent put message");

		if (!(rsp = nqc_msg_recv(nqc))) {
			err_log("Null response");
			err = merr_NULL_MESSAGE;
			goto cleanup;		
		}

		// increase seq num for response:
		msg_seq_incr(msg);	
		

		if (msg_cmd(rsp) == msg_OK) {
			err_log("message sent to queue");
			err = merr_NONE;
			goto cleanup;		
		} 

		if (msg_ko_action(rsp) == act_BYE) {
			err_log("message has not been sent to queue (%s)", msg_ko_message(rsp));
			err = merr_NONE;
			goto cleanup;

		} else {
			err_log("message has not been sent to queue (%s), retrying", msg_ko_message(rsp));
		}
		msg_destroy(rsp);
		msg_seq_incr(msg);	
	} 

	cleanup:
	if (msg) msg_destroy(msg);
	if (rsp) msg_destroy(rsp);
	
	return err;
}

int
nqc_put_file(pnqc_t nqc, char *filename) {
	merr_t err;
	char *data;
	size_t size;

	err_debug("nqc_put_file %s", filename);

	if (!(data = futl_load(filename, &size))) {
		err_error("cannot read file '%s'", filename);
		return 1;
	}
	err = nqc_put(nqc, data, size);
	if (data) free(data);
	return err;
}

int
nqc_get(pnqc_t nqc, char **data, size_t *size) {
	int seq = 1;
	pmsg_t msg, erm, rsp;
	merr_t	err;

	if (!(msg = msg_get_new(seq))){
		err_log("cannot create message");
		return 1;
	}
	while (1) {
		*data = NULL;
		*size = 0;

		if ((err = nqc_msg_send(nqc, msg)) != merr_NONE) {
			err_log("cannot send message (%s)", msg_err_string(err)); 
			goto cleanup;
		} 
	
		err_debug("get message sent, wait for a put...");
		
		if (!(rsp = nqc_msg_recv(nqc))) {
			err_log("Null response");
			err = merr_NULL_MESSAGE;
			goto cleanup;		
		}

		err_debug("got %s message", msg_cmd_string(msg_cmd(rsp)));

		// increase seq num for response:
		msg_seq_incr(msg); seq++;	

		if (msg_cmd(rsp) == msg_PUT) {
			err_log("received data from queue");
			unsigned long crc;
			err = merr_NONE;
			*data = msg_put_data(rsp);
			*size = msg_put_size(rsp);
			crc   = crc_32((unsigned char *) *data, *size);	

			if (crc != msg_put_crc(rsp)) {
				err = merr_PARTIAL_MESSAGE;
				nqc_msg_send(nqc, erm = msg_ko_new(seq++, err, act_RETRY, "invalid CRC"));
				msg_seq_incr(msg);
				if (erm) erm = msg_destroy(erm);	
			} else {
				err = merr_NONE;
				nqc_msg_send(nqc, erm = msg_ok_new(seq++));
				goto cleanup;		
			}

		} else if (msg_cmd(msg) != msg_KO) {
			err_error("invalid message (%s instead of %s)", msg_cmd_string(msg_cmd(msg)), msg_cmd_string(msg_KO));
			err = merr_INVALID_PROTOCOL;
			nqc_msg_send(nqc, erm = msg_ko_new(seq++, err, act_BYE, msg_err_string(err)));
			goto cleanup;	
		} else if (msg_ko_action(rsp) == act_BYE) {
			if (msg_ko_err(rsp) == merr_EMPTY) {
				err = merr_NONE;
				err_log("queue empty");
				goto cleanup;
			}
			err = msg_ko_err(rsp);
			goto cleanup;
		} else {
			err = msg_ko_err(rsp);
			err_log("error fetching data (%s), retrying", msg_ko_message(rsp));
		} 
		if (rsp) rsp = msg_destroy(rsp);
		if (erm) erm = msg_destroy(erm);
		msg_seq_incr(msg);
	}
	
	cleanup:
	if (msg) msg = msg_destroy(msg);
	if (rsp) rsp = msg_destroy(rsp);
	if (erm) erm = msg_destroy(erm);

	return err;
}

int
nqc_get_file(pnqc_t nqc, char *filename) {
	merr_t err;
	char *data;
	size_t size;
	
	if ((err = nqc_get(nqc, &data, &size) != merr_NONE)) {
		if (err == merr_EMPTY) {
			err_log("empty queue");
			return 0;
		} else {
			err_error("Problem retreiving data (%s)", msg_err_string(err));
			return 1;
		}
	}
	if (data) { 
		futl_write(filename, data, size);
		free(data);
	} else {
		err_log("no data");
	}

	return 0;
}
