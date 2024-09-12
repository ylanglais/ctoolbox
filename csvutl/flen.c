#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#include <tbx/mem.h>
#include <tbx/err.h>
#include <tbx/map.h>
#include <tbx/re.h>
#include <tbx/stral.h>
#include <tbx/hash.h>

typedef enum {
	ft_err,
	ft_null,
	ft_bool,
	ft_int,
	ft_float,
	ft_string
} ft_type;

static char *ft_types[] = { "error", "null", "bool", "int", "float", "string", "" };

char *CsV_type_str(ft_type t) {
	if (t < ft_err || t > ft_string) return "invalid type";
	return ft_types[t];
}

typedef struct {
	int nc;
	ft_type type;	
	char *name;
	int hasnull;
	phash_t  hash;
} _fld_t, *_pfld_t;


_pfld_t 
fld_destroy(_pfld_t fs, int n) {
	for (int i = 0; i < n; i++) {
		fs[i].hash = hash_destroy(fs[i].hash);
		if (fs[i].name) fs[i].name = mem_free(fs[i].name);
	}
	return NULL;
}

_pfld_t 
fld_new(int nb) {
	_pfld_t fs = malloc(nb * sizeof(_fld_t));
	memset(fs, 0, nb * sizeof(_fld_t));
	for (int i = 0; i < nb; i++) { fs[i].hash = hash_new(); }
	return fs;
}

typedef struct {
	char sep;
	char del;
} delsep_t;

typedef struct {
	char *fname;
	int  ro;
	char sep;
	char del;
	int crlf;
	int con1stline;
	pmap_t map;	
	char   *cur;
	char   *data;
	int    nfields;
	int    nlines;
	int    ncplm;
	_pfld_t fields;
	char   **index;
	pstral_t stral;
} CsV_t, *pCsV_t;

static pmap_t
__CsV_map_ro(char *fname) {
	pmap_t m;
	if ((m = map_new(fname, mapRDONLY))) return m;
	return NULL;
}

/***
char *
CsV_field_next(pCsV_t c) {
} 
**/

void __CsV_dmp(pCsV_t c) {
	if (!c) {
		err_error("null CsV");
		return;
	}
	printf("fname     : %s\n", c->fname);
	printf("ro        : %d\n", c->ro);
	printf("sep       : %c\n", c->sep);
	printf("del       : %c\n", c->del);
	printf("crlf      : %d\n", c->crlf);
	printf("con1stline: %d\n", c->con1stline);
	printf("map       : %p\n", c->map);
	printf("cur       : %p\n", c->cur);
	printf("data      : %p\n", c->data);
	printf("nlines    : %d\n", c->nlines);
	printf("ncplm     : %d\n", c->ncplm);
	printf("nfields   : %d\n", c->nfields);
	for (int i = 0; i < c->nfields; i++) {
		printf("fields[%d]: \n", i);
		if (c->fields[i].name) printf("\tname   : %s\n", c->fields[i].name);
		printf("\ttype   : %s\n", CsV_type_str(c->fields[i].type));
		printf("\tnc     : %d\n", c->fields[i].nc);
		printf("\thasnull: %d - %3.f%%\n", c->fields[i].hasnull, c->fields[i].hasnull * 100. / (c->nlines - c->con1stline));
		printf("\tuniques: %d - %3.f%%\n", hash_used(c->fields[i].hash), hash_used(c->fields[i].hash) * 100. / (c->nlines - c->con1stline));
	}
}

char *basenamecsv(char *s) {
	char *p;
	p = strdup(s);
	int l = strlen(s);
	if (strcmp(p+l-4, ".csv")) {
		err_error(p+l - 4);
		return p;
	}
	p[l-4] = 0;
	return p;	
}
void
CsV_create_table(pCsV_t c) {
	if (!c) return;
	char *s = basenamecsv(c->fname);
	printf("create table %s (\n", s);
	free(s);
	for (int i = 0; i < c->nfields; i++) {
		switch (c->fields[i].type) {
		case ft_bool:
		case ft_int:
			printf("\t%s int", c->fields[i].name);
			break;
		case ft_float:
			printf("\t%s float", c->fields[i].name);
			break;
		case ft_string:
			printf("\t%s varchar(%d)", c->fields[i].name, c->fields[i].nc + 1);
			break;
		default:
			printf("-- \t%s invalid type", c->fields[i].name);
	 	}

		if (hash_used(c->fields[i].hash) >= c->nlines - c->con1stline)
			printf(" unique");
		if (c->fields[i].hasnull == 0)
			printf(" not null");
		if (i < c->nfields - 1)
			printf(",");
		printf("\n");
	}
	printf(");\n");
}

void CsV_sql_import(pCsV_t c) {
	if (!c) return;
	char *s = basenamecsv(c->fname);
	printf("\\copy %s (",s); free(s);
	for (int i = 0; i < c->nfields; i++) {
		printf("%s", c->fields[i].name);
		if (i < c->nfields - 1)
			printf(", ");
	} 
	printf(") from '%s' csv delimiter '%c' quote '%c' encoding 'UTF-8'", c->fname, c->sep, c->del);
	if (c->con1stline) printf(" HEADER");
	printf(";\n");
}

size_t
CsV_sizeof() {
	return sizeof(CsV_t);
}

pCsV_t
CsV_destroy(pCsV_t c) {
	if (!c) 	   return c;
	if (c->fname)  c->fname  = mem_free(c->fname);
	if (c->fields) c->fields = fld_destroy(c->fields, c->nfields);
	if (c->stral)  c->stral  = stral_destroy(c->stral);
	if (c->index)  c->index  = mem_free(c->index);

	c->data   = NULL;
	c->map    = map_destroy(c->map);
	c         = mem_free(c);
	return NULL;
}

pCsV_t
CsV_new(char *fname, char sep, char del, int con1stline) {
	pCsV_t c = NULL;
	pmap_t m = NULL;
	if (!(m = __CsV_map_ro(fname))) return NULL;
	if (!(c = malloc(sizeof(CsV_t)))) {
		m = map_destroy(m);
		return NULL;
	}
	c->map        = m;
	c->fname      = strdup(fname); 
	c->sep        = sep;
	c->del        = del;
	c->cur        = 
	c->data       = map_data_get(c->map);
	c->ro         = 0;
	c->crlf       = 0;
	c->ncplm      = 0;
	c->nlines     = 0;
	c->con1stline = con1stline;
	c->index      = NULL;
	if (!(c->stral = stral_new())) {
		err_error("cannot create string allocator");
		return CsV_destroy(c);
	}
	return c;
}

char *
_CsV_scpy(char *from, char *to) {
	char *s, *p;
	if (!(s = (char *) malloc(to - from + 2))) return NULL;
	for (p = s; from <= to; *p++ = *from++);
	*p = 0;
	return s;
}

ft_type
CsV_guess_type(pCsV_t c, char *from, char *to) {
	if (from == NULL || to == NULL)                   return ft_err;
	if (from == to + 1)                               return ft_null;
	if (*from == c->del)                              return ft_string;
	if (from == to && (*from == '0' || *from == '1')) return ft_bool;
	if (!strchr("+-0123456789", *from))               return ft_string;
	ft_type type = ft_err;
	char *s = _CsV_scpy(from, to);
	if (!s)                                           return ft_err;
	pre_t pre;
	if (!(pre = re_new(s, "^(0|[+-]?[1-9][0-9]*)$", reEXTENDED))) 
		goto ret;
	if (re_find_first(pre)) { 
		type = ft_int; 
		goto ret; 
	}
	if (pre) pre = re_destroy(pre);
	if (!(pre = re_new(s, "^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$", reEXTENDED))) 
		goto ret;
	if (re_find_first(pre)) 
		type = ft_float; 

	type = ft_string;
ret:
	if (pre) pre = re_destroy(pre);
	if (s)   s   = mem_free(s);
	return type;
}

delsep_t 
CsV_guess_sep(pCsV_t c) {
	int maxl = 10;
	int minpchar = 33;
	int maxpchar = 126;
	int a[maxl][maxpchar - minpchar];
	int nl = 0;

	delsep_t ds = {0, 0};

	char *p = map_data_get(c->map);
	char *m = map_size_get(c->map) + p;

    char dels[] = "\"'`|&~-_@^=+$£*%!§:/.?";
	char *d;

	for (d = dels; d < dels + strlen(dels); d++) {
		memset(a, 0, sizeof(int) * (maxpchar - minpchar) * maxl);
		
		while (p < m && nl < maxl) {
			if (*d && *p == *d) while (*++p != *d && p < m);
			if (*p != '\n') a[nl][*p - minpchar]++;
			else {
				nl++; 
			}
			p++;
		}

		int i, j;
		for (j = minpchar; j < maxpchar; j++) {
			if (!isprint(j)) continue;	
			for (i = 1; i < nl; i++) { 
				if (a[i][j - minpchar] != a[i-1][j - minpchar]) break;
			}
			if (i == nl && a[i-1][j - minpchar] > 0) {
				ds.del = *d;
				ds.sep = j;
				return ds; 
			}
		}	
	}
	return ds;
}

void *
CsV_index_destroy(pCsV_t c) {
	if (!c) return NULL;
	if (c->index) c->index = mem_free(c->index);
	return NULL;
}

int
CsV_index_gen(pCsV_t c) {
	if (!c) return 1;
	c->index = (char **) malloc(c->nlines * sizeof(char*));

	char *p, *m;
	int i;

	p = map_data_get(c->map);
	m = map_size_get(c->map) + p;

	c->index[0] = p;
	
	for (i = 1; p < m; p++) {
		if (*p == c->del) while (*++p != c->del)
		if (c->sep > 0 && *p == '\n') {
			c->index[i++] = ++p;
		}
	}	
	return 0;
}

char *
CsV_namei(int i) {
	char b[50];
	sprintf(b, "column%d", i);
	return strdup(b);
}
	
void
CsV_analyse(pCsV_t c) {
	char *m, *p, *q;

	int ncpl  = 0;
	int ncpf  = 0;
	int nl    = 0;
	int nf    = 0;
	int nfc   = 0;
	int inq   = 0;

	/* count fields on first line: */
	p = map_data_get(c->map);
	m = map_size_get(c->map) + p;
	for (; p < m; p++) {
		if (*p == c->del) while (*++p != c->del);
		if (c->sep > 0 && (*p == c->sep || *p == '\n')) {
			nfc++;
			if (*p == '\n') break;
		}
	}

	c->nfields = nfc;
	c->fields  = fld_new(nfc);

	nfc = 0;	
	for (p = q = map_data_get(c->map); p < m; p++) {
		if (*p == c->del) while (*++p != c->del) ncpf++;
		if (c->sep > 0 && ( *p == c->sep || *p == '\n')) {
			c->fields[nfc].nc = 0;
			if (c->con1stline) c->fields[nfc].name = _CsV_scpy(q, p-1);
			else 			   c->fields[nfc].name =  CsV_namei(nfc);
			nfc++;
			q = p + 1;
			if (*p == '\n') break;
		}
	}

	nf   = 0;
	ncpl = 0;
	ncpf = 0;
	for (q = p = map_data_get(c->map); p < m; p++) {
		ncpl++;
		ncpf++;
		if (c->del >0 && *p == c->del) {
			if (inq) inq = 0;
			else     inq = 1;
		}
		if (!inq && (*p == c->sep || *p == '\n')) {
			unsigned long ul;

			if (nf < c->nfields && c->fields[nf].nc < ncpf) c->fields[nf].nc = ncpf - 1;
			if (!c->con1stline || c->nlines > 0) {
				char *ss, *s = NULL;
				ft_type t;
				t = CsV_guess_type(c, q, p - 1);
				
				if (t == ft_null) {
					c->fields[nf].hasnull++;
					s = strdup("NULL");	
				} else if (t > ft_null) {
					s = _CsV_scpy(q, p - 1);
				}
				ss = stral_dup(c->stral, s);	
				s = mem_free(s);
				if ((ul = (unsigned long) hash_retrieve(c->fields[nf].hash, ss))) {
					ul = ul + 1;
				} else {
					hash_insert(c->fields[nf].hash, ss, (void *) 1);
				}
	
				if (t > c->fields[nf].type) 
					c->fields[nf].type = t;
					//printf("%d -> type = %d (%s)\n", nf, c->fields[nf].type, CsV_type_str(c->fields[nf].type));
			}
			ncpf = 0;	
			nf++;
			q = p + 1;

			if (*p == '\n') {
				c->nlines++;
				if (p[1] == '\r') { 
					p++; c->crlf = 1; 
				} else if (nf != c->nfields) {
					err_warning("line %d: found %d fields instead of %d", nl, nf, c->nfields);
				}
				nf = 0;
				if (ncpl > c->ncplm) c->ncplm = ncpl;
				ncpl = 0;
			}
		}	
	}
}

int main(int n, char *a[]) {
	char sep = ';';
	char del = '"';
	char *tmp;

	int con1stline = 0, opt;

	if ((tmp = getenv("CSV_SEP")) )sep = *tmp;
	if ((tmp = getenv("CSV_DEL"))) del = *tmp;

	while ((opt = getopt(n, a, "cd:s:")) != -1) {
		switch (opt) {
        case 'c':
        	con1stline = 1;
			break;
		case 'd':
			del = *optarg;
            break;
		case 's':
			sep = *optarg;
            break;
		default: /* '?' */
			err_warning("'%c' is not a valid argument", opt);
		}
	}
	err_info("sep = '%c', del = '%c'", sep, del);
	int i;
	for (i = optind; i < n; i++) {
		pCsV_t c = NULL;
		err_info("process file %a");
		if ((c = CsV_new(a[i], sep, del, con1stline))) {
			delsep_t ds;
			ds = CsV_guess_sep(c);
			printf("guessed sep: '%c', guessed del: '%c'\n", ds.sep, ds.del);
			CsV_analyse(c);
			__CsV_dmp(c);
			printf("\n");

			CsV_create_table(c);
			CsV_sql_import(c);

			c = CsV_destroy(c);
		}
	}
	return 0;
}


