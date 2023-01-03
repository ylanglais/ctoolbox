#include <stdio.h>
#include <strings.h>
#include <string.h>

#include "atree.h"
#include "storage.h"
#include "rexp.h"
#include "mem.h"
#include "err.h"

#define optFMTSIZE  16
#define optCODESIZE 64
#define optLINESIZE 1024

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	30/05/2016  1.1  add tbx std tags
	04/08/2016  1.2  add changelog & tbx tags
	03/01/2023  1.3  small fixes
*/   

char opt_MODULE[]  = "Option file parser";
char opt_PURPOSE[] = "simple multi format option file parser";
char opt_VERSION[] = "1.3";
char opt_DATEVER[] = "03/01/2023";

typedef struct {
	char *format;
	p_rexp_t rexp;
	patree_t atree;
	pstorage_t storage;
} opt_t, *popt_t;

typedef struct {
	char name[optCODESIZE];
	char format[optFMTSIZE];
	char *var;
	size_t size;
	void (*set)(void *, char *);
} optent_t, *poptent_t;

#define _option_c_
#include "option.h"
#undef _option_c_

popt_t
opt_destroy(popt_t opt) {
	if (!opt) return NULL;
	if (opt->atree)   atree_destroy(opt->atree);
	if (opt->storage) storage_destroy(opt->storage);	
	if (opt->format)  mem_free(opt->format);
	if (opt->rexp)	  rexp_destroy(opt->rexp);
		
	mem_free(opt);
	return NULL;
}

size_t opt_sizeof() { return sizeof(opt_t); }

popt_t 
opt_new(char *format){
	popt_t opt;
	if (!format) return NULL;
	if (!(opt = (popt_t) mem_zmalloc(sizeof(opt_t)))) return NULL;
	if (!(opt->atree = atree_new())) return opt_destroy(opt);
	if (!(opt->storage = storage_new(sizeof(optent_t), 50))) return opt_destroy(opt);
	opt->format = mem_strdup(format);	
	if (!(opt->rexp = rexp_new(NULL, format, rexp_EXTENDED))) return opt_destroy(opt);
	return opt;
}

int
opt_entry_add(popt_t opt, char *name, char *format, size_t size, void *var, void (*set)(void *, char *)) {
	optent_t optent;	
	if (!opt) return 1;
	if (!name || strlen(name) < 1) return 2;
	
	memset(&optent, 0, sizeof(optent_t));
	strcpy(optent.name, name);
	if (format) strcpy(optent.format, format);
	optent.var  = var;
	optent.size = size;
	optent.set  = set;
	atree_store(opt->atree, name, storage_add(opt->storage, (char *) &optent));
	return 0;
}

int
opt_parse(popt_t opt, char *filename, void *data) {
	FILE *f = NULL;
	char *p, *code, *value, line[optLINESIZE];
	poptent_t ent;
	
	
	if (!opt) return 1;
	if (!filename || !strcmp(filename, "-")) f = stdin;
	else if (!(f = fopen(filename, "r"))) return 1;	
	while (!feof(f)) {
		fgets(line, optLINESIZE, f);
		p = line;
		while (*p == ' ' || *p == '\t') p++;
		if (*p == '#') continue;
		rexp_buffer_set(opt->rexp, line);
		if (!rexp_find(opt->rexp)) continue;
		code  = rexp_sub_get(opt->rexp, 1);
		value = rexp_sub_get(opt->rexp, 2);

		/* cleanup value by triming on the right: */
		p = value;
		while (*p) p++;
		p--;
		while (*p == ' ' || *p == '\t' || *p == '\n') p--;
		/* cleanup value by triming on the left: */
		p++; *p = 0;
		p = value; 
		while (*p == ' ' || *p == '\t') ++p;
		
		err_debug("code(%s) -> \"%s\"", code, p);

		ent = atree_retrieve(opt->atree, code);
		if (ent) {
			if      (ent->set)    ent->set(data, p);
			else if (ent->format[0]) sscanf(p, ent->format, ent->var);
			else if (ent->size)   strncpy(ent->var, p, ent->size);
			else err_debug("code(%s) -> \"%s\"", code, p);
		} else {
			err_debug("opt parse: unknown code: %s", code);
		}
		mem_free(code); mem_free(value);
	}
	if (f != stdin) fclose(f);
	return 0;	
}
