
/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005-2011, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 * 
 *  Changelog:
 *  09/10/2017	1.0	Creation
 *  31/05/2018	1.1	fix mem alloc for ending 0 byte in b64_decode
 */

#include <stdlib.h>
#include <string.h>

#include "b64.h"

char b64_MODULE[]  = "base 64 {en,de}coding from Jouni Malinen";
char b64_PURPOSE[] = "base 64 {en,de}coding from Jouni Malinen";
char b64_VERSION[] = "1.1";
char b64_DATEVER[] = "31/05/2018";


static const unsigned char b64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * b64_encode - Base64 encode
 * @src: Data to be encoded
 * @len: Length of the data to be encoded
 * @out_len: Pointer to output length variable, or %NULL if not used
 * Returns: Allocated buffer of out_len bytes of encoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer. Returned buffer is
 * nul terminated to make it easier to use as a C string. The nul terminator is
 * not included in out_len.
 */
#define LW 74
unsigned char *
b64_encode(const unsigned char *src, size_t len, size_t *out_len) {
    unsigned char *out, *pos;
    const unsigned char *end, *in;
    size_t olen;
    int line_len;

    olen = len * 4 / 3 + 4;     /* 3-byte blocks to 4-byte */
    olen += olen / LW;          /* line feeds */
    olen++;                     /* nul termination */
    if (olen < len)
        return NULL;            /* integer overflow */
    out = malloc(olen);
    if (out == NULL)
        return NULL;

    end = src + len;
    in = src;
    pos = out;
    line_len = 0;
    while (end - in >= 3) {
        *pos++ = b64_table[in[0] >> 2];
        *pos++ = b64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = b64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = b64_table[in[2]   & 0x3f];
        in += 3;
        line_len += 4;
        if (line_len >= LW) {
            *pos++ = '\n';
            line_len = 0;
        }
    }

    if (end - in) {
        *pos++ = b64_table[in[0] >> 2];
        if (end - in == 1) {
            *pos++ = b64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        } else {
            *pos++ = b64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *pos++ = b64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
        line_len += 4;
    }

    if (line_len)
        *pos++ = '\n';

    *pos = '\0';
    if (out_len)
        *out_len = pos - out;
    return out;
}

/**
 * b64_decode - Base64 decode
 * @src: Data to be decoded
 * @len: Length of the data to be decoded
 * @out_len: Pointer to output length variable
 * Returns: Allocated buffer of out_len bytes of decoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer.
 */
unsigned char *
b64_decode(const unsigned char *src, size_t len, size_t * out_len) {
    unsigned char dtable[256], *out, *pos, block[4], tmp;
    size_t i, count, olen;
    int pad = 0;

    memset(dtable, 0x80, 256);
    for (i = 0; i < sizeof(b64_table) - 1; i++)
        dtable[b64_table[i]] = (unsigned char)i;
    dtable['='] = 0;

    count = 0;
    for (i = 0; i < len; i++) {
        if (dtable[src[i]] != 0x80)
            count++;
    }

    if (count == 0 || count % 4)
        return NULL;

    olen = count / 4 * 3;
    pos = out = malloc(olen + 1);
    if (out == NULL)
        return NULL;

    count = 0;
    for (i = 0; i < len; i++) {
        tmp = dtable[src[i]];
        if (tmp == 0x80)
            continue;

        if (src[i] == '=')
            pad++;
        block[count] = tmp;
        count++;
        if (count == 4) {
            *pos++ = (block[0] << 2) | (block[1] >> 4);
            *pos++ = (block[1] << 4) | (block[2] >> 2);
            *pos++ = (block[2] << 6) | block[3];
            count = 0;
            if (pad) {
                if (pad == 1)
                    pos--;
                else if (pad == 2)
                    pos -= 2;
                else {
                    /* Invalid padding */
                    free(out);
                    return NULL;
                }
                break;
            }
        }
    }

    *out_len = pos - out;
    return out;
}

#ifdef _test_b64_

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "err.h"
#include "futl.h"

int main(void) {
	size_t len;
	char *stro;
	char *strb64sys;
	char *strb64tbx;
	char *strf;
	char name[100];
	char cmd[100];

	err_level_set(err_DEBUG);

	/* Read source file: */
	if (!(stro = futl_load(__FILE__, NULL))) return 1;
	
	sprintf(name, ".tmp.%s", __FILE__);
	sprintf(cmd, "base64 %s > %s", __FILE__, name);
	system(cmd);

	if (!(strb64sys = futl_load(name, NULL))) {
		free(stro);
		return 2;
	}
	strb64tbx = (char *) b64_encode((unsigned char *) stro, strlen(stro), &len);
	
	if (strcmp(strb64sys, strb64tbx)) {
		err_error("encoder diverges: ");
		err_error("\n%s", strb64tbx);
	} else {
		err_debug("encoder seems fine");
	}
		
	strf = (char *) b64_decode((unsigned char *) strb64sys, strlen(strb64sys), &len);
	if (strcmp(stro, strf)) {
		err_error("decoder diverges");
	} else {
		err_debug("decoder seems fine");
	}
	
	free(stro); free(strf);
	free(strb64sys); free(strb64tbx);

	return 0;
}
#endif

