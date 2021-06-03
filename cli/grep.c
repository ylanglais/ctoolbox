#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tbx/re.h>
#include <tbx/map.h>

enum {
	norm,
	bold,
	black,
	red,
	green,
	yellow,
	blue,
	magenta,
	cyan,
	white
};

char *_colors[white + 1];
#define color_get(col) _colors[col] = getenv(#col)
void colors() {
	color_get(norm);
	color_get(bold);
	color_get(black);
	color_get(red);
	color_get(green);
	color_get(yellow);
	color_get(blue);
	color_get(magenta);
	color_get(cyan);
	color_get(white);
}

void usage(char *a) {
	printf("%s usage:\n%s pattern file...\n");
}

char *substr(char *b, int len) {
	char *p, *q;
	if (!(p = malloc(len + 2))) return NULL;

	for (q = p; q <= p + len; *q++ = *b++) ;
	*q = 0;
	return p;
}

char *sol(char *b, char *p) {
	char *q;
	for (q = p; q > b && *q != '\n'; q--);
	if (*q == '\n') q++;
	
	return substr(q, p - q - 1);
}

char *solp(char *b, char *p) {
	char *q;
	for (q = p; q > b && *q != '\n'; q--);
	if (*q == '\n') q++;
	
	return q;
}

char *eol(char *p) {
	char *q;
	for (q = p; *q && *q != '\n'; q++);
	if (*q == '\n') q--;
	if (q > p) p++;
	return substr(p, q - p);
}

char *eolp(char *p) {
	char *q;
	if (!p) return 0;
	for (q = p; *q && *q != '\n'; q++);
	return q;
}

int line_count(char *b, char *p) {
	int c;
	char *q;

	for (q = b, c = 0; q < p; q++) if (*q == '\n') c++;
	return c;
}

int match_all(pre_t r, char *filename) { 
	char *b, *s, *e, *p, *lp;
	int  lc = 1;
	prematch_t m;

	lp = p = b = re_buffer(r);
	while ((m = re_find(r, p))) {
		s = solp(b, p + m->subs[0].so);
		e = eolp(p);
		lc += line_count(lp, p); 
		printf("%s %d: ", filename, lc); fflush(stdout);
	soloop:
		while (p < lp + m->subs[0].so) { putchar(*p++); fflush(stdout); }
		printf("%s%s", _colors[yellow], _colors[bold]); fflush(stdout);
		while (p < lp + m->subs[0].eo) {
			if (*p == '\n') {
					lc++;
					printf("\n%s%s:%c : %s%s", _colors[norm], filename, lc, _colors[yellow], _colors[bold]); fflush(stdout);
					e = eolp(p + 1); 
			} else { 
				putchar(*p++); fflush(stdout); 
			}
		}
		printf("%s", _colors[norm]); fflush(stdout);
		if ((m = re_find(r, p)) && e > p + m->subs[0].so) {
			lp = p;
			goto soloop;
		} else while (p <= e) { putchar(*p++); fflush(stdout); }
		lc++;
		lp = p;
	}
	return 0;
}

int main(int n, char *a[]) {
	int i;
	int mode = 0;
	pmap_t map;
	pre_t r;
 
	if (!strcmp(a[0], "egrep")) mode |= reEXTENDED;
	
	if (n < 3) {
		usage(a[0]);
		return 1;
	}

	if (!(r = re_new(a[1], "a", mode))) {
		return 2;
	}	
	re_destroy(r);
	
	colors();
	
	for (i = 2; i < n; i++) {
		if (!(map = map_new(a[i], mapRDONLY))) {
			fprintf(stderr, "%s error: cannot open %s\n", a[0], a[i]);
		} else {
			char *p;

			p = map_data_get(map);
			r = re_new(p, a[1], mode);

			match_all(r, a[i]); 

			r = re_destroy(r);
			map_destroy(map);
		}
	}
	return 0;
}
