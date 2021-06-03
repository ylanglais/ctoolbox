#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "err.h"
#include "mem.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	18/07/2016 	1.0	 Creation
	04/08/2016  1.3  add separator in parameter & take text separator into account for counting fields.
*/   

char csv_MODULE[]  = "CSV file reader";
char csv_PURPOSE[] = "Simple french formatted CVS file reader (incomplete)";
char csv_VERSION[] = "1.3";
char csv_DATEVER[] = "04/08/2016";

typedef struct {
	char 	filename[1024];
	char    sep;
	size_t 	maxlinesize;
	size_t  nfields;
	size_t	nlines;
	char 	**map;
	char 	data;
} csv_t, *pcsv_t;

#define _csv_c_
#include "csv.h"
#undef _csv_c_

pcsv_t 
csv_destroy(pcsv_t c) {
	if (c) {
		if (c->map) free(c->map);
		c->map = NULL;
		free(c);
	}
	return NULL;
}

static char *
_csv_field_set(pcsv_t c, int i, int j, char *str) {
	if (!c || !c->map) return NULL;
	return c->map[i * c->nfields + j] = str;
}

pcsv_t 
csv_new(char *filename, char separator) {
	int f, i, j;
	char *p, *q, *sol;
	size_t fin;
	pcsv_t c;
	struct stat stats;

 	if (!filename) return NULL;
	if (stat(filename, &stats)) return NULL;
	if (!(c = (pcsv_t) mem_zmalloc(sizeof(csv_t) + stats.st_size - 1))) return NULL;
	c->map = NULL;
	if (!(f = open(filename, O_RDONLY))) { 
		err_error("cannot open %s : %s\n", filename, strerror(errno));
		return csv_destroy(c);
	}
	if (!read(f, &c->data, stats.st_size)) {
		err_error("error reading %s : %s\n", filename, strerror(errno));
		return csv_destroy(c);
	}
	fin = (long) ((char *) &c->data) + stats.st_size;

	if (!strchr(";|,:\t ", separator)) c->sep = ';';

	else c->sep = separator;
	c->nlines = c->nfields = 0;
	p = &c->data;
	while (*p != '\n' && (long) p < fin) {
		/* check if field starts by ": */
		if (*p == '"' && (p == &c->data || *(p-1) == c->sep)) while (*++p &&  *p != '"');
		if (*p++ == c->sep) c->nfields++;
	}
	while ((long) p < fin) if (*p++ == '\n') c->nlines++;
	if (!c->nfields || !c->nlines) {
		err_error("%f is an empty file\n", filename);
		return csv_destroy(c);
	}
	if (!(c->map = mem_zmalloc(c->nfields * (c->nlines + 1) * sizeof(char *)))) {
		return csv_destroy(c);	
	}
	c->maxlinesize = 0;
	i = j = 0;
	for (sol = q = p = &c->data; (long) p < fin; p++) {
		if (*p == c->sep) {
			_csv_field_set(c, i, j++, q); 
			q = p + 1; 
			*p = 0;	
		} else if (*p == '\n') {
			_csv_field_set(c, i++, j++, q); 
			if ((long) (p - sol) > c->maxlinesize) c->maxlinesize = (size_t) (p - sol);
			sol = q = p + 1; 
			*p = 0;
			j = 0;
			if (p[-1] == '\r' ) p[-1] = 0;
		}
	}
	close(f);
 	return c; 
}

int
csv_lines(pcsv_t c) {
	if (!c) return -1;
	return c->nlines;
}

int 
csv_fields(pcsv_t c) {
	if (!c) return -1;
	return c->nfields;
}

char *
csv_line(pcsv_t c, int i) {
	char *line, *p, *q;
	int j;
	if (!c) return NULL;
	if (!(line = (char *) mem_zmalloc(c->maxlinesize))) return NULL;

	/* I know what I did, but since it's tricky, chances are that I will not remember in a short while :

	   Objective of this line is to concatenate all the fields of a line into a new string:
	   j counts the number of fields,
	   p points to the current char of the new string,
	   q points to the current char of the line.

		init triple folded but easy;
		stop condition is simple : we need to have all the fields; 
		increment part : we copied a whole field with while loop. We need to replace the 0 char
						 by a separator and update the field count (j);

		while loop : strcpy as implemented in pointer part shown in K&R C Reference Language. 
	 */

	for (j = 0, p = line, q = c->map[i]; j <= c->nfields; *(p - 1) = c->sep, j++) while ((*p++ = *q++)); 

	/* remove trailing sep (previously pointed by p): */
	p[-1] = 0;

	return line;
}

char *
csv_field_get(pcsv_t c, int i, int j) {
	if (!c) return NULL;
	return  c->map[i * c->nfields + j];
}

char *
csv_field_get_by_name(pcsv_t c, int i, char *fieldname) {
	if (!c) return NULL;
	int j;
	for (j = 0; j < c->nfields && strcmp(fieldname,c->map[j]); j++);
	if (j < c->nfields) return csv_field_get(c, i, j);
	return NULL;
}

#ifdef _test_csv_
int 
main(void) {
	char *b[][40] = { 
		{"#champ", "offset", "len", "iskey", "omit"},
		{"CTRJEU", "1", "1", "n", "n"},
		{"CIDFPR", "2", "1", "n", "n"},
		{"CNRDIA", "3", "3", "n", "n"},
		{"DAEMIN", "6", "7", "n", "n"},
		{"FILLER1", "13", "1", "n", "y"},
		{"CANNIN", "14", "1", "n", "n"},
		{"FILLER2", "15", "36", "n", "y"},
		{"DTARIN", "51", "7", "n", "n"},
		{"NSOCIE", "58", "4", "n", "n"}
	};
	char *l;
	int i, j;
	FILE *f;
	
	pcsv_t c;

	f = fopen("test-csv.csv", "w");
	for (i = 0; i < 10; i++) {
		fprintf(f, "%s;%s;%s;%s;%s\r\n", b[i][0],b[i][1],b[i][2],b[i][3],b[i][4]);
	} 
	fclose(f);

	c = csv_new("test-csv.csv", ';');
	for (i = 0; i < csv_lines(c); i++) for (j = 0; j < csv_fields(c); j++) {
		if (strcmp(b[i][j], csv_field_get(c, i, j))) {
			printf("b[%d][%d] = \"%s\", csv_field_get(%x, %d, %d) = \"%s\"\n",
				i, j, b[i][j], c, i, j, csv_field_get(c, i, j));
		} else {
			printf("%d:%d : Ok\n", i, j);
		}
	} 
	printf("line(0) = \"%s\"\n", l = csv_line(c, 0));
	printf("field(1, \"#champ\") = %s\n", csv_field_get_by_name(c, 1, "#champ"));
	printf("field(2, \"offset\") = %s\n", csv_field_get_by_name(c, 2, "offset"));
	 
	csv_destroy(c);
	return 0;
}
#endif

