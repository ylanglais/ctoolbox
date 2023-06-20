#include <stdlib.h>
#include <stdio.h>
#include <mem.h>
#include <coverage.h>
#define NAMESIZE 64
#define RGBFILE "rgb.txt"
#define loops   1000

typedef struct {
	char *name;
	int r, g, b;
} rgb_t, *prgb_t;

#include <hash.hh>
#include <storage.hh>
#include <err.h>

int main(void) {
	int i, n, j;	
	FILE *f;
	char name[NAMESIZE], *cols[800], *t;
	
	rgb_t *color;

	struct timeval tv, tmp;

	Hash *h;
	Storage *s;

	if (!(h = new Hash(1000))) {
		printf("cannot create hash table\n");
		exit(1);
	}
	
	if (!(s = new Storage(sizeof(rgb_t), 100))) {
		printf("cannot create storage\n");
		exit(2);
	}

	if (!(f = fopen(RGBFILE, "r"))) {
		printf("cannot open %s file\n", RGBFILE);
		exit(3);
	}

	printf("read rgb data from %s\n", RGBFILE);

	i = 0;
	while (!feof(f)) {
		rgb_t rgb;
		void *p;

		memset(name, 0, NAMESIZE);
		if (fscanf(f, "%d %d %d\t%[^\n]", &rgb.r, &rgb.g, &rgb.b, name) == 4) { 
			cols[i] = rgb.name = mem_strdup(name);
			p = s->add((char *) &rgb);
			h->insert(cols[i], (char *) p);
			i++;
		}
	}	
	n = s->used();

	printf("getting from hash with %d slots :", h->size()); fflush(stdout);
	tv = cov_time_zero();
	for (j = 1; j < loops; j++) 
    for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		h->retrieve(cols[i]);
		tv = cov_time_add(tv, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done in %s seconds\n", t =  cov_time_fmt(tv)); fflush(stdout);
	
	h->stats(); 

	printf("rehashing with %d slots...", h->used());
	h->rehash(h->used());
	printf("done \n"); fflush(stdout);
	
	printf("getting from hash with %d slots : ", h->size()); fflush(stdout);
	tv = cov_time_zero();
	for (j = 1; j < loops; j++) 
	for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		h->retrieve(cols[i]);
		tv = cov_time_add(tv, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done in %s seconds\n", t =  cov_time_fmt(tv)); fflush(stdout);
	h->stats(); fflush(stdout);

	
	color = (prgb_t) (*h)["linen"];
	if (!color) {
		printf("cannot retrieve color\n");
	} else {
		printf("color[\"%s\"] = (%d, %d, %d)\n", color->name, color->r, color->g, color->b);
	}

	return 0;
}

