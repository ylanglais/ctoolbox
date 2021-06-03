
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "mdbg_utils.h"

//static char bintable[] = "01";
//static char octtable[] = "01234567";
static char dectable[] = "0123456789";
static char hextable[] = "0123456789abcdef";

char *
mdbg_fmt_ptr(char *s, void *ptr) {
	int i;
	long l = (long) ptr;
	char *p;

	if (!s) return s;
	p = s;
	for (i = 0; i < sizeof(ptr); i++) *p++ = '0', *p++ = '0';
	*p-- = 0;
	for (i = 0; i < sizeof(ptr); i++, l >>= 8) {
		*p-- = hextable[l & 0x0F];
		*p-- = hextable[(l & 0xF0) >> 4];
	}
	return s;
}

char *
mdbg_fmt_char(char *s, char c) {
	if (s) {
		s[0] = isprint(c) ? c : '.';
		s[1] = 0;
	}
	return s;
}

char *
mdbg_fmt_xchar(char *s, char c) {
	if (s) {
		s[1] = hextable[(c & 0x0F)     ];
		s[0] = hextable[(c & 0xF0) >> 4];
	}
	s[2] = 0;
	return s;
}

char *
mdbg_fmt_u(char *s, unsigned int ui) {
	int i, j, n;
	if (!s) return s;

	for (n = 0, j = 1; ui / j; n++, j *= 10);

	j /= 10; 

	for (i = 0; i < n; i++) {
		s[i] = dectable[ui / j];
		ui -= (ui / (unsigned int) j) * j;
		j /= 10;
	}
	if (i == 0) s[i++] = '0';
	s[i] = 0;
	return s;
}

char *
mdbg_fmt_u_n(char *s, unsigned int ui, int m, char pad) {
	int i, j, n;
	char *s2;

	if (!s) return s;
	s2 = s;
	for (n = 0, j = 1; ui / j; n++, j *= 10);

	j /= 10; 

	if (n < m) for (i = n; i < m; i++) *s2++ = pad;

	for (i = 0; i < n; i++) {
		s2[i] = dectable[ui / j];
		ui -= (ui / (unsigned int) j) * j;
		j /= 10;
	}

	if (i == 0)  s2[i-1] = '0';
	s2[i] = 0;

	if (n >= m) s[m] = 0;

	return s;
}


char *
mdbg_str_apend(char *s, char *t) {
	if (!s) return s;
	for (; *s; s++);
	if (!t) return s;
	while ((*s++ = *t++));
	return --s;
}
