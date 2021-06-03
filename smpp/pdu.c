/*
	This code is released under ...  terms. 
	Please read license terms and conditions at ... 

	Yann LANGLAIS

	Changelog:
	13/03/2013 0.9.0 initial release 
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

#include <tbx/err.h>

#include <pdu.h>

size_t 
pdu_len(ppdu_t p) {
	if (!p) return 0;
	return ntohl(p->command_len);
}

int
pdu_cmd(ppdu_t p) {
	if (!p) return 0;
	return ntohl(p->command_id);	
}

int
pdu_seq(ppdu_t p) {
	if (!p) return 0;
	return ntohl(p->seq_num);	
}

int
pdu_status(ppdu_t p) {
	if (!p) return -1;
	return ntohl(p->command_status);	
}

void
pdu_hexdump(ppdu_t p) {
	int i, j;
	char hex[256], asc[256], tmp[1000];
	char *buff;
	size_t size;

	if (!getenv("SMPPDBG")) return;

	size = ntohl(p->command_len);
	buff = (char *) p;

	if (!p) return;
	err_debug("pdu content:");
	for (i = 0; i < size; i += 16) {
		hex[0] = asc[0] = '\0';
		for (j = 0; j < 16 && i + j < size; j++) {
			if (j == 8) {
				strcat(hex, "- ");
				strcat(asc, " - ");
			}
			sprintf(tmp, "%02x ", (unsigned char) buff[i+j]);
			strcat(hex, tmp);
			sprintf(tmp, "%c", isprint(buff[i + j]) ? buff[i + j] : '.');
			strcat(asc, tmp);
		}
		err_debug("%-50s | %-19s", hex, asc);
	}
}
