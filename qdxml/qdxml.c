#include "qdxml.h"

int
qdxml_parse(char *data, char *buf, size_t size, start_f start, stop_f stop, value_f value) {
	char *end, t, *p, *q;

	if (!buf || !start || !stop || !value) return 1;	

	for (p = buf, end = buf + size; p < end; p++) {
		if (*p == '<') 	{
			if (p[1] == '?') { /* skip version/fmt header */
				p++;
				while (*++p != '?');
				p++;
				continue;
			}

			q = ++p;

			if (*q == '/') {
				/* closing tag : */
				while (*q != '>') q++;
				t  = *q;
				*q = 0;

				stop(data, ++p);

				*q = t;
				p = q - 1;

			} else {

				while (*q != '>' && *q != ' ' && *q != '\t' && *q != '\n') {
					if (*q == '\'')  while (*++q != '\'');
					if (*q == '"')   while (*++q != '"');
					q++;
				}

				if (*q !=  '>') {
					char *pt;

					t = *q;

					for (pt = q; *pt != '>'; pt++);
					if (pt[-1] == '/') {
						t = *q;
						/* this is an empty tag... */
						*q = 0; 

						start(data, p);
						//value(data, "");
						stop(data, p);

						*q = t;
						q = pt;
					} else {
						t = *q;
						*q = 0;

						start(data, p);

						*q = t;
						q = pt;
					}	
				} else {
				
					if (q[-1] == '/') {
						q[-1] = 0;
						start(data, p);
						//value(data, "");
						stop(data, p);
						q[-1] = '/';

					} else {
						
						*q = 0;

						start(data, p);

						*q = '>';
					}
				}
				p = q - 1;
			}
		} else if (*p == '>') {
			if (p[1] == '<') continue;
			if (p[-1] != '/') {
				++p;
				while (*p == ' ' || *p == '\t' || *p == '\n') p++;
				q = p;
				while (*q && *q != '<') q++;
				*q = 0;
				if (*p) value(data, p);
				*q = '<';
				p = q - 1; 
			}
		}	
	}
	return 0;
}

