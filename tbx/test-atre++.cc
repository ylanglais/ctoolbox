#include <stdio.h>
#include <string.h>
#include <storage.hh>
#include <atree.hh>
#define RGBFILE "rgb.txt"
#include <mem.h>


typedef struct {
	int r, g, b;
} rgb_t, *prgb_t;

extern "C" void rgb_print(void *p) {
	prgb_t pr;
	pr = (prgb_t) p;
	printf("(%03d, %03d, %03d)", pr->r, pr->g, pr->b);
}

int main(void) {
	Atree *pa;
	Storage *ps;
	char *cols[752];
	int i;
	char name[128];
	FILE *pf;

	printf("atr test\n");

	ps = new Storage(sizeof(rgb_t), 100);
	pa = new Atree();

	if (!ps || !pa) return 33;

	pf = fopen(RGBFILE, "r");
	printf("store rgb data from %s\n", RGBFILE);
	
	i = 0;
	while (!feof(pf)) {
		rgb_t rgb;
		memset(name, 0, 128);
		if (fscanf(pf, "%d %d %d\t%[^\n]", &rgb.r, &rgb.g, &rgb.b, name) == 4) { 
			cols[i] = mem_strdup(name);
			pa->store(cols[i], (char *) ps->add((char *) &rgb)); 
			i++;
		}
		/* printf("*********\n%d\n", i); */
		/* atr_dump(pa, rgb_print); */
	}
	fclose(pf);

	printf("\n*********\natr dump:\n");
	pa->dump(rgb_print);

	/* final chech: */
	printf("\n*********\nretrieval\n");
	for (i = 0; i < ps->used(); i++) {
		prgb_t prgb0, prgb1;
		prgb0 = (prgb_t) ps->get(i);
		prgb1 = (prgb_t) pa->retrieve(cols[i]);
		printf("%d --> %s: (%d, %d, %d) /// (%d, %d, %d)\n", 
				   i, cols[i], 
				   prgb0->r, prgb0->g, prgb0->b,
				   prgb1->r, prgb1->g, prgb1->b);
		mem_free(cols[i]);
	}

	return 0;
}

