#include <stdlib.h>
#include <string.h>

#include <tbx/err.h>
//#include <tbx/option.h>

#include "nqc.h"

int main(int n, char *a[])  {
	int i;
	pnqc_t nqc;
	//popt_t opt;
	char *conffile = NULL;
	char *host     = NULL;
	char *file     = NULL;
	int   port     = 0;
	int   debug    = 0;

	//err_init(NULL, err_WARNING);
	err_init(NULL, err_DEBUG);

	/* Parse command line : */
	for (i = 1; i < n; i++) {
		if (!strcmp(a[i], "-c")) {
			conffile = a[++i];			
		} else if (!strcmp(a[i], "-h")) {
			host   = a[++i];
			err_debug("host = %s", host);
		} else if (!strcmp(a[i], "-p")) {
			port   = atoi(a[++i]);
			err_debug("port = %d", port);
		} else if (!strcmp(a[i], "-f")) {
			file   = a[++i];
			err_debug("file = %s", file);
		} else if (!strcmp(a[i], "-d")) {
			debug = 1;
			err_debug("debug = %d", debug);
			err_init(NULL, err_DEBUG);
		} else {
			if (a[i][0] != '-') {
				file = a[i];
				err_debug("file = %s", file);
			} else {
				err_error("unknown arguement %s", a[i]);
			}
		}
	}
#if 0
	
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
	
	opt_parse(opt, conffile, &nqs);
	opt_destroy(opt);

	if (!(nqs.nq = nq_new(nqs.path))) exit(merr_BAD_QUEUE);

	/* Prepare loging: */
	char buf[1000];	
	sprintf(buf, "%s/nqs.log", nqs.path);
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
	}
#endif
	
	if (!host) {
		err_error("queue host undefined");
		return 1;	
	} 
	if (!port) {
		err_error("queue port undefined");
		return 2;
	}
	if (!file) {
		err_error("no filename given");
	}

	nqc = nqc_new(host, port);
	if        (!strcmp("nqput", basename(a[0]))) {
		nqc_put_file(nqc, file);
	} else if (!strcmp("nqget", basename(a[0]))) {
		nqc_get_file(nqc, file);
	} else {
		err_error("unknown nq command %s", basename(a[0]));
	}
	nqc = nqc_destroy(nqc);
	
	return 0;
}
