
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <tbx/err.h>

typedef struct {
	char task_command[4100];
} tt_t;

static tt_t _dstcli_;

char **
dstcli_task_args_prepare() {
	int  n, i;
	char **a, *p, *q, c;
	
	if (!_dstcli_.task_command || !*_dstcli_.task_command) {
		err_error("null task command");
		return NULL;
	}

	/* count args */
	
	q = p = _dstcli_.task_command;
	n = 0;
	while (*p) {
		switch (*p) {
		case '"':
			while (*++p != '"' && *p);
			if (!*p) {
				err_error("unbalanced '\"' in command line : '%s'", q);
				return NULL; 
			}
			break;
		case '\'':
			while (*++p != '\'' && *p);
			if (!*p) {
				err_error("unbalanced \"'\" in command line : '%s'", q);
				return NULL; 
			}
			break;
		case '`':
			while (*++p != '`' && *p);
			if (!*p) {
				err_error("unbalanced \"`\" in command line : '%s'", q);
				return NULL; 
			}
			break;
		case ' ':
		case '\t':
			if (*(p - 1) != '\\') { 
				n++;
				p++;
				while (*p == ' ' || *p == '\t') p++;
				--p;	
			}
			break;
		}
		p++;
		if (!*p && (*(p-1) != ' ' && *(p-1) != '\t') && *(p-2) != '\\') n++;
	}


	if (!(a = (char **) malloc((n + 1) * sizeof(char *)))) {
		err_error("cannot allocate memory for argv array");
		return NULL;
	}	
	
	a[n] = NULL;

	p = q;
	i = 0;
	
	/* TODO: split args */
	while (*p) {
		switch (*p) {
		case '"':
			while (*++p != '"' && *p);
			if (!*p) {
				err_error("unbalanced '\"' in command line : '%s'", q);
				return NULL; 
			}
			break;
		case '\'':
			while (*++p != '\'' && *p);
			if (!*p) {
				err_error("unbalanced \"'\" in command line : '%s'", q);
				return NULL; 
			}
			break;
		case '`':
			while (*++p != '`' && *p);
			if (!*p) {
				err_error("unbalanced \"`\" in command line : '%s'", q);
				return NULL; 
			}
			break;
		case ' ':
		case '\t':
			if (*(p - 1) != '\\') {
				c = *p;
				*p = 0;
				a[i++] = strdup(q);
				*p = c;
				p++;
				while (*p == ' ' || *p == '\t') p++;
				q = --p;	
				while (*q == ' ' || *q == '\t') q++; 
			}
		
			break;
		}
		p++;
		if (!*p && (*(p-1) != ' ' && *(p-1) != '\t') && *(p-2) != '\\') a[i++] = strdup(q);
	}

	for (i = 0; i < n; i++) err_debug("a[%d] = %s", i, a[i]);

	return a;
}

void
argv_dump(char *a[]) {
	int i;

	if (!a) {
		err_error("pointeur null");
		return;
	}

	printf(" a = %x\n", a);
	printf("*a = %s\n", *a);

	for (i = 0; a[i]; i++) 
		printf("a + %d = %x, *(a+%d) = %s\n", i, a + i, i, *(a+i));
	
}


int 
main(int n, char *a[], char *e[]) {
	char **argv;

	err_level_set(err_DEBUG);
	sprintf(_dstcli_.task_command, "toto0 titi1 2 trois");
	err_debug("dstcli_task_args_prepare(\"%s\") =", _dstcli_.task_command);
	argv = dstcli_task_args_prepare();
	argv_dump(argv);
	if (argv) free(argv);

	sprintf(_dstcli_.task_command, "toto0  titi1 2	 \"t r o i s\" quatre   ");
	argv = dstcli_task_args_prepare();
	err_debug("dstcli_task_args_prepare(\"%s\") =", _dstcli_.task_command);
	argv_dump(argv);
	if (argv) free(argv);
	
	sprintf(_dstcli_.task_command, "toto0 titi1 2 \"t r o i s\" 'qua - tre' cinq");
	err_debug("dstcli_task_args_prepare(\"%s\") =", _dstcli_.task_command);
	argv = dstcli_task_args_prepare();
	argv_dump(argv);
	if (argv) free(argv);
		
	sprintf(_dstcli_.task_command, "toto0 titi1 2 \"t r o i s\" 'qua - tre' `c i n q`\n");
	err_debug("dstcli_task_args_prepare(\"%s\") =", _dstcli_.task_command);
	argv = dstcli_task_args_prepare();
	argv_dump(argv);
	if (argv) free(argv);

	sprintf(_dstcli_.task_command, "toto0 titi1 2 	\"t r o i s\"\\ 'qua - tre' `c i n q`");
	err_debug("dstcli_task_args_prepare(\"%s\") =", _dstcli_.task_command);
	argv = dstcli_task_args_prepare();
	argv_dump(argv);
	if (argv) free(argv);

	sprintf(_dstcli_.task_command, "toto0 titi1 2 	\"t r o i s\\ 'qua - tre' `c i n q`");
	err_debug("dstcli_task_args_prepare(\"%s\") =", _dstcli_.task_command);
	argv = dstcli_task_args_prepare();
	argv_dump(argv);
	if (argv) free(argv);

	sprintf(_dstcli_.task_command, "sleep 12");
	err_debug("dstcli_task_args_prepare(\"%s\") =", _dstcli_.task_command);
	argv = dstcli_task_args_prepare();
	argv_dump(argv);

	if (argv) free(argv);
	sprintf(_dstcli_.task_command, "sleep");
	err_debug("dstcli_task_args_prepare(\"%s\") =", _dstcli_.task_command);
	argv = dstcli_task_args_prepare();
	argv_dump(argv);
	if (argv) free(argv);
		
/* 
	printf(" e = %x\n", e);
	printf("*e = %x\n", *e);
	for (i = 0; e[i]; i++) 
		printf("e + %d = %x, e[%d] = %s\n", i, e + i, i, e[i]);
*/

	return 0;
}
	
