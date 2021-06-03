/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
		20/09/2017	1.0	Creation
    
*/   

char zprs_MODULE[]  = "Zlib compression/uncompression";
char zprs_PURPOSE[] = "zlib compression/uncompression";
char zprs_VERSION[] = "1.0";
char zprs_DATEVER[] = "20/09/2017";


#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "err.h"
#include "zprs.h"

#define zprsLEVEL 6

char *
_zprs_error_string[] = {
	"Ok",
	"No memory",
	"Reserved buffer too small",
	"Corrupted data",
	"Undocumented Zlib error"
};
	
enum {
	zprsOK,
	zprsNOMEM,
	zprsBUFF,
	zprsCORR,
	zprsUNDOC
};


char *
zprs_err_str(int errn) {
	switch (errn) {
	case Z_OK:
		return _zprs_error_string[zprsOK];
	case Z_MEM_ERROR:
		return _zprs_error_string[zprsNOMEM];
	case Z_BUF_ERROR:
		return _zprs_error_string[zprsBUFF];
	case Z_DATA_ERROR:
		return _zprs_error_string[zprsCORR];
	default:
		return _zprs_error_string[zprsUNDOC];
	}
}

char *
zprs_try_realloc(char *data, size_t size) {
	char *dest;
	if (!(dest = (char *) malloc(size))) {
		err_info("Never mind, but could not realloc correct amount if memory (%lu)", size);
		return data;
	}
	memcpy(dest, data, size);
	free(data);
	return dest;
}

static char *
_zprs_compress(char *data, size_t *size, int level) {
	char *dest;
	size_t dsize;
	int    errn;
	if (level < 0 || level > 9) {
		err_info("Never mind, but compression level of %d is not possible (only from 0 to 9) and therefore reset to 6", level);
		level = 6;
	}

	dsize = compressBound(*size);

	if (!(dest = (char *) malloc(dsize))) {
		*size = 0;
		err_error("not enough memory");
		return NULL;
	}
	if ((errn = compress2((Bytef *) dest, (uLongf *) &dsize, (const Bytef *) data, (uLongf) *size, level)) != Z_OK) {
		err_error("cannot compress (%s) buffer sized %lu", zprs_err_str(errn), dsize);
		free(dest);
		*size = 0;
		return NULL;
	}
	*size = dsize;
	return zprs_try_realloc(dest, dsize);
}

char *
zprs_compress(char *data, size_t *size) {
	return _zprs_compress(data, size, zprsLEVEL);
}

char *
zprs_compress_level(char *data, size_t *size, int level) {
	return _zprs_compress(data, size, level);
}

char *
zprs_uncompress(char * data, size_t *size) {
	size_t dsize;
	char   *dest;
	dsize = *size * 5;
	int errn;
		
	retry:	
	if (!(dest = malloc(dsize))) {
		*size = 0;
		err_error("not enough memory");
		return NULL;
	}
		
	errn = uncompress((Bytef *) dest, (uLongf *) &dsize, (const Bytef *) data, (uLong) *size);
	if (errn == Z_BUF_ERROR) {
		dsize *= 2;	
		free(dest);
		goto retry;
	}

	if (errn != Z_OK) {
		err_error("cannot ucompress (%s)", zprs_err_str(errn));
		free(dest);
		*size = 0;
		return NULL;
	}

	*size = dsize;
	return zprs_try_realloc(dest, dsize);
}

#ifdef _test_zprs_ 
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

extern int errno;

int main(int n, char *a[]) {
	int f;
	struct stat stats;
	size_t size;
	char *input, *output, *check;

	err_level_set(err_DEBUG);

	if (n < 2) {
		err_error("%s must be given a file in order to create a compressed version", a[0]);
		return 1;
	}

	if (stat(a[1], &stats)) {
		err_error("cannot stat file %s", a[1]);
		return 2;
	}

	size = stats.st_size;

	if (!(input = malloc(size))) {
		err_error("cannot allocate %lu bytes", size);
		return 3;
	}

	if (!(f = open(a[1], O_RDONLY))) { 
		err_error("cannot open %s : %s\n", a[1], strerror(errno));
		free(input);
		return 4;
	}
	if (!read(f, input, size)) {
		err_error("error reading %s : %s\n", a[1], strerror(errno));
		free(input);
		return 5;
	}

	close(f);

	if (!(output = zprs_compress(input, &size))) {
		err_error("could not compress data");
		free(input);
		return 6;	
	}
	
	err_info("compressed %s from %lu to %lu (%4.2f)", a[1], stats.st_size, size, (double) size / (double) stats.st_size);

	char *fname;
	if (!(fname = malloc(strlen(a[1]) + 5 + 1))) {
		err_error("cannot allocate mem for output file");
		free(input); free(output);
		return 7;
	}
	
	sprintf(fname, "%s.zlib", a[1]);
	
	if ((f = open(fname, O_WRONLY | O_SYNC | O_CREAT | O_TRUNC, 00640)) < 0) {	
		err_error("cannot open output file '%s'", fname);
		free(input); free(output);
		return 8;
	}	

	if ((write(f, output, size)) != size) {
		err_warning("partial writing");
	}
	
	close(f);

	if (!(check = zprs_uncompress(output, &size))) {
		err_error("cannot uncomptress");
		free(input); free(output);
		return 9;
	};

	sprintf(fname, "%s.chck", a[1]);
	
	if ((f = open(fname, O_WRONLY | O_SYNC | O_CREAT | O_TRUNC, 00640)) < 0) {	
		err_error("cannot open output file '%s'", fname);
	} else {	
		if ((write(f, check, size)) != size) {
			err_warning("partial writing");
		}
		close(f);
	}

	if (size != stats.st_size) {
		err_error("incorrect size found %lu (awaited %lu)", size, stats.st_size);
		free(input); free(output); free(check);
		return 10;
	}

	if (memcmp(input, check, size)) {
		err_error("input and check differ");
		free(input); free(output); free(check);
		return 11;
	}

	err_info("input and check are identical");	
	free(input); free(output); free(check);
	return 0;
}
#endif
