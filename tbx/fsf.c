#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "mem.h"
#include "err.h"
#include "storage.h"
#include "hash.h"
#include "csv.h"
#include "map.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
		30/05/2016 1.1	add changelog
*/

char fsf_MODULE[]  = "Simple fixed size data file parser";
char fsf_PURPOSE[] = "Parse fixed sized data files";
char fsf_VERSION[] = "1.1";
char fsf_DATEVER[] = "30/05/2016";

typedef struct {
	char name[64];
	size_t offset;
	size_t len;
	char   iskey;
	char   omit;
} fsfdef_t, *pfsfdef_t;

typedef struct {
	pstorage_t spec;
	phash_t		nameidx;
	int 	   *kidxlst;
	size_t 		nkeys;
	size_t 		nfields;
	size_t		keysize;
	size_t		lsize;
	pstorage_t 	data;
} fsf_t, *pfsf_t;

#define _fsf_c_
#include "fsf.h"
#undef _fsf_c_

pfsf_t
fsf_destroy(pfsf_t s) {
	if (s) {
		if (s->kidxlst) free(s->kidxlst);
		if (s->spec)    s->spec    = storage_destroy(s->spec);
		if (s->data)    s->data    = storage_destroy(s->data);
		if (s->nameidx) s->nameidx = hash_destroy(s->nameidx);
		free(s);
	}
	return NULL;
}

int
fsf_lines(pfsf_t s) {
	if (!s) return 0;
	return storage_used(s->data);
}

int
fsf_fields(pfsf_t s) {
	if (!s) return 0;
	return s->nfields;
}

char *
fsf_field_get(pfsf_t s, int i, int j) {
	char *l, *f;
	pfsfdef_t d;

	if (!s || !s->data) return NULL;

	if (i > storage_used(s->data)) {
		err_warning("%d is too big for a line number (max %d)", i, storage_used(s->data));
		return NULL;
	}
	if (j > storage_used(s->spec)) {
		err_warning("%d is too big for a column number (max %d)", j, storage_used(s->spec));
		return NULL;
	}
	
	l = (char *)    storage_get(s->data, i);
	d = (pfsfdef_t) storage_get(s->spec, j);
	
	if (!(f = (char *) mem_zmalloc(d->len + 1))) {
		err_warning("no memory for field (%d, %d, named %s)", i, j, d->name);
		return NULL;
	}
	strncpy(f, l + d->offset, d->len);
	f[d->len] = 0;
	return f;
}

pfsf_t
fsf_new(char *specfile, char *datafile) {
	int i, nk = 0;
	char *    line;
	char *    p;
	char *    last;
	pcsv_t    csv;
	pfsf_t    s = NULL;
	pfsfdef_t d = NULL;
	pmap_t    m = NULL;

	if (!specfile || !*specfile) {
		err_error("invalid spec filename");
		return fsf_destroy(s);
	}
	if (!datafile || !*datafile) {
		err_error("invalid data filename");
		return fsf_destroy(s);
	}
	
	if (!(csv = csv_new(specfile, ';'))) {
		err_error("error reading spec file \"%s\"", specfile);
		return fsf_destroy(s);
	}
	
	if (!(s = (pfsf_t) mem_zmalloc(sizeof(fsf_t)))) {
		err_error("cannot allocate memory for fsf data");
		return fsf_destroy(s);
	}

	if (!(s->spec = storage_new(sizeof(fsfdef_t), 20))) {
		err_error("cannot create storage for fsf spec");
		return fsf_destroy(s);
	}

	if (!(s->nameidx = hash_new())) {
		err_error("cannot create hash for fsf spec");
		return fsf_destroy(s);
	}

	for (i = 0; i < csv_lines(csv); i++) {
		char *p;
		fsfdef_t defl;

		strncpy(defl.name, csv_field_get(csv, i, 0), 64); defl.name[63] = 0;
		defl.offset = atol(csv_field_get(csv, i, 1));
		defl.len    = atol(csv_field_get(csv, i, 2));
		p = csv_field_get(csv, i, 3);	
		defl.iskey = (*p == 'y' ? 1 : 0);
		if (defl.iskey) nk++;
		p = csv_field_get(csv, i, 4);	
		defl.omit  = (*p == 'y' ? 1 : 0);
		if (!(d = (pfsfdef_t) storage_add(s->spec, (char *) &defl))) {
			err_error("error reading spec line %d (\"%s\")", csv_line(csv, i));
			csv = csv_destroy(csv);	
			return fsf_destroy(s);
		}
		hash_insert(s->nameidx, d->name, (void *) (unsigned long) i);
	}
	s->nfields = i;
	csv = csv_destroy(csv);	
	
	if (!(s->kidxlst = (int *) mem_zmalloc(nk * sizeof(int)))) {
		err_error("cannot retreive last line from the spec");
		return fsf_destroy(s);
	}

	s->nkeys = 0;
	for (i = 0; i < s->nfields; i++) {
		if (!(d = (pfsfdef_t) storage_get(s->spec, i))) continue;
		if (d->iskey) {
			s->kidxlst[s->nkeys++] = i; 
			s->keysize += d->len;
		}
	}
	s->lsize = d->offset + d->len + 1;

	if (!(d = (pfsfdef_t) storage_get(s->spec, storage_used(s->spec)))) {
		err_error("cannot retreive last line from the spec");
		return fsf_destroy(s);
	}
	
	if (!(s->data = storage_new(s->lsize, 100))) {
		err_error("cannot create storage for fsf data");
		return fsf_destroy(s);
	}

	if (!(m = map_new(datafile, mapRDONLY))) {
		return fsf_destroy(s);
	}

	i = 0;
	line = NULL;	

	last = (p = map_data_get(m)) + map_size_get(m);
	while (p < last) {
		char *q;
		if (!(line = (char *) mem_zmalloc(s->lsize))) {
			err_error("no more memory while reading line %d", i + 1);
			map_destroy(m);
			return fsf_destroy(s);
		}
		for (q = line; *p != '\n' && p < last && (q - line) < s->lsize; *q++ = *p++);	

		/* Check if line is ok: */
		if (*q || *p) {

			if (p == last) {
				/* EOF has no \n, add it: */
				*q = 0;
			} else if (*p != '\n') { 
				/* Line is too long: */
				err_info("line %d is too long (%d chars) and is truncated to %d\n", i + 1, (int) (q - line),  s->lsize);

				/* Skip no next eol: */
				while (*q++ == *p++ && p < last);
			} else p++;
		} 
		if ((q - line) < s->lsize - 1) {
			err_warning("line %d is too short (%d chars instead of %d)\n", i + 1, (q - line), s->lsize);
		}
		
		/* Fix CRLF to LF: */
		if (q[-1] == '\r') q[-1] = 0;
		
		char *l;

		if (!(l = storage_add(s->data, line))) {
			err_warning("cannot insert line \"%s\"", line); 
		} 
		if (line) line = mem_free(line);
		i++;
	}	
	err_info("read %d lines and stored %d lines", i, storage_used(s->data));

	map_destroy(m);
	return s;
}

char *
fsf_field_get_by_name(pfsf_t s, int i, char *name) {
	if (!s || !s->nameidx) return NULL;
	return fsf_field_get(s, i, (int)(unsigned long) hash_retrieve(s->nameidx, name));
}

char *
fsf_line_get(pfsf_t s, int i) {
	if (!s || !s->data) return NULL;
	return (char *) storage_get(s->data, i);
}

#ifdef _test_fsf_

int 
main(int na, char *a[]) {
	int i, l, n;
	pfsf_t f;
	char *c;

	err_init(NULL, err_INFO);
	f = fsf_new("tests/fsf.testdataset.spec", "tests/fsf.testdataset.data");

	if (na > 1)
		l = atoi(a[1]);	
	else 
		l = 9;

	n = fsf_lines(f);

	//printf("%d: %s\n", l, fsf_line_get(f, l));
	printf("%d: LREFDO = \"%s\"\n", l, c = fsf_field_get_by_name(f, 1, "LREFDO"));
	if (c) free(c);
	for (i = 0; i < n; i++) {
		printf("l:%d,c:%d: %s\n", i, l, c = fsf_field_get(f, i, l));
		if (c) free(c);
	}
	fsf_destroy(f);

	return 0;
}

#endif
