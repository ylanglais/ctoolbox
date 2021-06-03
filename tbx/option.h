#ifndef _option_h_
#define _option_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _option_c_
typedef struct _opt_t *popt_t;
#endif

size_t opt_sizeof();

popt_t opt_destroy(popt_t opt);
popt_t opt_new(char *format);
int opt_entry_add(popt_t opt, char *name, char *format, size_t size, void *var, void (*set)(void *, char *));
int opt_parse(popt_t opt, char *filename, void *data);

#if 0 
EXAMPLE:
/* define a configuration structure: */
typedef struct {
	int     dim;
	int     size;
	int     level;
	double  phy;
	double  theta;
	double  Rmodifier;
	int     nrules;
	prule_t rules;
	char    root[lsysTOSIZE];
	double randpcent;
} conf_t, *pconf_t;

/* define a parsing function for something called "rule" which looks like : "%c -> %s" 
 * function/callback prototype must be as follow:  void (*)(void *, char *) 
 * the void pointer is the pointer to configuration structure 
 * and the char * points to the string to be interpreted 
 */

rule_add(void *p, char *str) {
	pconf_t pc;
	
	pc = (pconf_t) p;
	if (!pc) return;
	if (pc->nrules <= 0) {
		pc->rules = (prule_t) mem_malloc(sizeof(rule_t));
	} else {
		pc->rules = (prule_t) mem_realloc(pc->rules, (pc->nrules + 1) * sizeof(rule_t));
	}
	sscanf(str, "%c -> %s", &pc->rules[pc->nrules].from, &pc->rules[pc->nrules].to);
	pc->nrules++; 
}

{

	/* ... */

    conf_t conf;
	popt_t opt; /* --> create a option struct pointer */
	pstorage_t store;
	
	if (n < 2) {
		fprintf(stderr, "%s usage:\n%s conffile\n", a[0], a[0]);
		return 1;
	}

	srand(getpid());
	
	/* initialize configuration structure (set to zero): */
	memset(&conf, 0, sizeof(conf_t));
	/* default required non null config parameters: */
	conf.dim = 2;
	
	/* prepare the opt structure: allocate memory AND set the config file line format: */
	/* (ident+:+separator(s)+Value+separators+newline) */
	opt = opt_new("([A-z0-9_]+):[ \t]+([^ \t].*[^\n\t ]*)[ \t\n]*");

	/* register all possible identifiers w/ their format, size, dest pointer and/or specific parsing function */
	opt_entry_add(opt, "dim",       "%d",       sizeof(int),    &conf.dim,       NULL);
	opt_entry_add(opt, "size",      "%d",       sizeof(int),    &conf.size,      NULL);
	opt_entry_add(opt, "level",     "%d",       sizeof(int),    &conf.level,     NULL);
	opt_entry_add(opt, "phy",       "%lf",      sizeof(double), &conf.phy,       NULL);
	opt_entry_add(opt, "theta",     "%lf",      sizeof(double), &conf.theta,     NULL);
	opt_entry_add(opt, "Rfactor",   "%lf",      sizeof(double), &conf.Rmodifier, NULL);
	opt_entry_add(opt, "rule",      "%c -> %s", 0,              NULL,            rule_add);
	opt_entry_add(opt, "root",      "%s",       lsysTOSIZE,     conf.root,       NULL);
	opt_entry_add(opt, "randpcent", "%lf",      sizeof(double), &conf.randpcent, NULL);

	/* open option/config file named a[1]. Pointer to conf struct will be passed to specific parsing functions: */
	opt_parse(opt, a[1], &conf);

	/* destroy opt data: */
	opt_destroy(opt);

#endif

#ifdef __cplusplus
}
#endif

#endif
