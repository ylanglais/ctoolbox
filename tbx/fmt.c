
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#include "fmt.h"


/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	04/08/2016  3.1  add separator in parameter & take text separator into account for counting fields.
	30/10/2017  3.2  add a fmt_bin w/ malloced data for "normal" programs & bugfgix
*/   

char fmt_MODULE[]  = "Stack only simple data to string formatter w/o memory allocation";
char fmt_PURPOSE[] = "Stack only simple data to string formatter w/o memory allocation";
char fmt_VERSION[] = "3.2";
char fmt_DATEVER[] = "30/10/2017";

//static char bintable[] = "01";
//static char octtable[] = "01234567";
static char dectable[] = "0123456789";
static char hextable[] = "0123456789abcdef";

static int	fmtfd = -1;

char *
fmt_ptr(char *s, void *ptr) {
	int i;
	long l = (long) ptr;
	char *p;

	if (!s) return s;
	p = s;
	for (i = 0; i < sizeof(ptr); i++) *p++ = '0', *p++ = '0';
	*p-- = 0;
	for (i = 0; i < sizeof(ptr); i++, l >>= 8) {
		*p-- = hextable[ l & 0x0F];
		*p-- = hextable[(l & 0xF0) >> 4];
	}
	return s;
}

char *
fmt_char(char *s, char c) {
	if (s) {
		s[0] = isprint(c) ? c : '.';
		s[1] = 0;
	}
	return s;
}

char *
fmt_xchar(char *s, char c) {
	if (s) {
		s[1] = hextable[(c & 0x0F)     ];
		s[0] = hextable[(c & 0xF0) >> 4];
	}
	s[2] = 0;
	return s;
}

char *
fmt_u(char *s, unsigned int ui) {
	unsigned long i, j, n;
	if (!s) return s;

	j = 1;
	for (n = 0; ui / j > 0; n++) {
		j *= 10;
	}

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
fmt_u_n(char *s, unsigned int ui, int m, char pad) {
	unsigned long i, j, n;
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

char *fmt_xd(char *s, double d) {
	static int n = sizeof(double);
	int i;
	char *p, *q;

	if (!s) return NULL;
	p = (char *) &d;
	q = s;
	for (i = n - 1; i >= 0; i--) {
		*q++ = hextable[(p[i] & 0xF0) >> 4];
		*q++ = hextable[(p[i] & 0x0F)     ];
	}
	*q = 0;
	return s;
}

char *fmt_xld(char *s, long double d) {
	static int n = sizeof(long double);
	int i;
	char *p, *q;
	if (!s) return NULL;
	p = (char *) &d;
	q = s;
	for (i = n - 1; i >= 0; i--) {
		*q++ = hextable[(p[i] & 0xF0) >> 4];
		*q++ = hextable[(p[i] & 0x0F)     ];
	}
	*q = 0;
	return s;
}

char *
fmt_str_append(char *s, char *t) {
	if (!s) return s;
	for (; *s; s++);
	if (!t) return s;
	while ((*s++ = *t++));
	return --s;
}

void 
fmt_stderr_print(char *s) {
	if (s) write(2, s, strlen(s));
}

void 
fmt_stdio_print(char *s) {
	if (s) write(1, s, strlen(s));
}

void
fmt_fd_set(int fd) {
	if (fd <= 0) fd = 2;
	fmtfd = fd;
}

void
fmt_print(char *s) {
	if (fmtfd < 0) fmtfd = 2;
	if (s) write(fmtfd, s, strlen(s));
}

void 
fmt_dump_bin_data(char *buff, size_t size) {
    int i, j;
    char hex[256], asc[256], tmp[512];
	char *h, *a;

    if (buff == NULL) {
       	fmt_print("null pointer\n");
        return;
    }

    fmt_print("block dump  :\n");
	
    /* 00 11 22 33 44 55 66 77 - 88 99 AA BB CC DD EE FF   azertyui - qsdfghjk */
    for (i = 0; i < size; i += 16) {
        *hex = *asc = 0;
		h = hex, a = asc;
        for (j = 0; j < 16 && i + j < size; j++) {
            if (j == 8) {
                h = fmt_str_append(h, "- ");
                a = fmt_str_append(a, " - ");
            }
			h = fmt_str_append(h, fmt_xchar(tmp, (unsigned char) buff[i+j]));
			h = fmt_str_append(h, " ");
			a = fmt_str_append(a, fmt_char(tmp, buff[i + j]));
        }
		if (j < 16) {
			while (j <= 16) { 
				j++;
				if (j == 0) h = fmt_str_append(h, "- ");
				h = fmt_str_append(h, "   ");
			}
		}
 
		fmt_print("    ");	
		fmt_print(fmt_ptr(tmp, buff + i)); 
		fmt_print(": ");
		fmt_print(hex);
		fmt_print("  ");
		fmt_print(asc);
        fmt_print("\n");
    }
}

char *
fmt_bin(char *data, size_t size) {
	char *final = NULL, *p;
	size_t sz;
	
    int i, j;
    char hex[256], asc[256], tmp[512];
	char *h, *a;

	if (size <= 0) return NULL;

	sz = 95 * (size / 16 + 1);
	if (!(final = malloc(sz))) return NULL;
	memset(final, 0, sz);
		
	for (p = final, i = 0; i < size; i += 16) {
        *hex = *asc = *tmp = 0;
		h = hex, a = asc;
        for (j = 0; j < 16 && i + j < size; j++) {
            if (j == 8) {
                h = fmt_str_append(h, "- ");
                a = fmt_str_append(a, " - ");
            }
			h = fmt_str_append(h, fmt_xchar(tmp, (unsigned char) data[i+j]));
			h = fmt_str_append(h, " ");
			a = fmt_str_append(a, fmt_char(tmp, data[i + j]));
        }
		if (j < 16) {
			while (j <= 16) { 
				j++;
				if (j == 0) h = fmt_str_append(h, "- ");
				h = fmt_str_append(h, "   ");
			}
		}
 
		sprintf(p, "    %x: %s  %s\n", data + i, hex, asc);
		while (*++p);
    }
	return final;
}

#ifdef _test_fmt_
#define PI 3.141592653589793238462643383279502884L
int main(void) {
	char tmp[100]; 
	char tst[] = "azertyuiop¨£¤\n\t123456789\nAABBCCDD\n";
	printf("ptr:   %p -> %s\n", 0xdeadbeaf, fmt_ptr(tmp, (void *) 0xdeadbeaf));
	printf("char:  %c -> %s\n", 'a', fmt_char(tmp, 'a'));
	printf("xchar: %c -> %s\n", 'a', fmt_xchar(tmp, 'a'));
	printf("uint:  %u -> %s\n", 1234567890, fmt_u(tmp, 1234567890));
	printf("u_17:  %u -> %s\n", 1234567890, fmt_u_n(tmp, 1234567890, 17, ' '));
	printf("u_17:  %u -> %s\n", 1234567890, fmt_u_n(tmp, 1234567890, 17, '0'));
	printf("xd:    %f -> %s\n", (double) PI, fmt_xd(tmp, (double) PI));
	printf("xld:   %Lf -> %s\n", (long double) PI, fmt_xld(tmp, (long double) PI));
	fmt_dump_bin_data(tst, strlen(tst));
	return 0;
}
#endif
