#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include "storage.h"
#include "coverage.h"
#include "hash.h"
#include "index.h"
#include "atree.h"
#include "atr.h"
#include "patr.h"
#include "btree.h"
#include "mem.h"

#define RGBFILE "rgb.txt"
#define NAMESIZE 64
#define loops   100000

#define tstruct struct timeval

typedef struct {
	char *name;
	int r, g, b;
}rgb_t, *prgb_t;

int cmp(void *a, void *b) {
	if (a && b) 
		return strcmp(((prgb_t) a)->name, ((prgb_t) b)->name);
	return 0;
}

void rgb_print(void *rgb) {
	if (rgb)
		printf("%20s (% 3d, % 3d, % 3d)\n", ((prgb_t) rgb)->name, ((prgb_t) rgb)->r, ((prgb_t) rgb)->g, ((prgb_t) rgb)->b); 
}

int main(int na, char **a) {
	int i, j, n;
	pstorage_t ps;
	phash_t ph;
	pindex_t pi;
	patree_t pa;
	patr_t pa2;
	ppatr_t ppa;
	pbtree_t pb;
	FILE *f;
	char name[NAMESIZE], *cols[800];
	struct timeval  acreate, a2create, pacreate, bcreate, hcreate, icreate, astore, a2store, pastore, bstore, hstore, istore,
					aget, a2get, paget, bget, hget, iget, afree, a2free, pafree, bfree, hfree, ifree, tmp;

	if (!(ps = storage_new(sizeof(rgb_t), 100))) exit(1);

	acreate   = cov_time_get();
	if (!(pa  = atree_new())) exit(2);
	acreate   = cov_time_sub(cov_time_get(), acreate);

	a2create  = cov_time_get();
	if (!(pa2 = atr_new())) exit(2);
	a2create  = cov_time_sub(cov_time_get(), a2create);

	pacreate  = cov_time_get();
	if (!(ppa = patr_new("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_ "))) exit(2);
	pacreate  = cov_time_sub(cov_time_get(), pacreate);

	bcreate   = cov_time_get();
	if (!(pb  = btree_new(cmp))) exit(2);
	bcreate   = cov_time_sub(cov_time_get(), bcreate);

	hcreate   = cov_time_get();
	if (!(ph  = hash_new_sized(1000))) exit(3);
	hcreate   = cov_time_sub(cov_time_get(), hcreate);

	icreate   = cov_time_get();
	if (!(pi  = index_new())) exit(3);
	icreate   = cov_time_sub(cov_time_get(), icreate);
	

	/* storing */
	if (!(f = fopen(RGBFILE, "r"))) {
		printf("cannot open \"%s\" file, exiting\n", RGBFILE);
		storage_destroy(ps);
		atree_destroy(pa);
		atr_destroy(pa2);
		patr_destroy(ppa);
		btree_destroy(pb);
		hash_destroy(ph);
		index_destroy(pi);
		exit(1);
	}
	printf("read rgb data from %s\n", RGBFILE);

	astore = a2store = pastore = bstore = hstore = istore = cov_time_zero();
	i = 0;
	while (!feof(f)) {
		rgb_t rgb;
		void *p;

		memset(name, 0, NAMESIZE);
		if (fscanf(f, "%d %d %d\t%[^\n]", &rgb.r, &rgb.g, &rgb.b, name) == 4) { 
			cols[i] = rgb.name = mem_strdup(name);
			p = storage_add(ps, (char *) &rgb);

			tmp = cov_time_get();
			atree_store(pa, cols[i], (char *) p); 
			astore = cov_time_add(astore, cov_time_sub(cov_time_get(), tmp));

			tmp = cov_time_get();
			atr_store(pa2, cols[i], (char *) p); 
			a2store = cov_time_add(a2store, cov_time_sub(cov_time_get(), tmp));

			tmp = cov_time_get();
			patr_store(ppa, cols[i], (char *) p); 
			pastore = cov_time_add(pastore, cov_time_sub(cov_time_get(), tmp));

			tmp = cov_time_get();
			btree_store(pb, (void *) p); 
			bstore = cov_time_add(bstore, cov_time_sub(cov_time_get(), tmp));

			tmp = cov_time_get();
			hash_insert(ph, cols[i], (char *) p);
			hstore = cov_time_add(hstore, cov_time_sub(cov_time_get(), tmp));

			tmp = cov_time_get();
			index_add(pi, cols[i], (char *) p);
			istore = cov_time_add(istore, cov_time_sub(cov_time_get(), tmp));

			i++;
		}
	}
	fclose(f);
	n = storage_used(ps);
	
	/* btree_print_raw(pb, rgb_print); */
	/* btree_print_sorted(pb, rgb_print); */

	/* getting: */
	printf("getting w/ atree..."); fflush(stdout);
	aget = cov_time_zero();
	for (j = 1; j < loops; j++) 
    for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		atree_retrieve(pa, cols[i]);
		aget = cov_time_add(aget, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done\n"); fflush(stdout);
	
	printf("getting w/ atr..."); fflush(stdout);
	a2get = cov_time_zero();
	for (j = 1; j < loops; j++) 
    for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		atr_retrieve(pa2, cols[i]);
		a2get = cov_time_add(a2get, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done\n"); fflush(stdout);

	printf("getting w/ patr..."); fflush(stdout);
	paget = cov_time_zero();
	for (j = 1; j < loops; j++) 
    for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		patr_retrieve(ppa, cols[i]);
		paget = cov_time_add(paget, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done\n"); fflush(stdout);

	printf("getting w/ btree..."); fflush(stdout);
	bget = cov_time_zero();
	for (j = 1; j < loops; j++) 
    for (i = 0; i < n; i++) {
		void *o;
		o = storage_get( ps, i);
		tmp = cov_time_get();
		btree_goto(pb, o);
		bget = cov_time_add(bget, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done\n"); fflush(stdout);
	
	printf("getting w/ hash..."); fflush(stdout);
	hget = cov_time_zero();
	for (j = 1; j < loops; j++) 
    for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		hash_retrieve(ph, cols[i]);
		hget = cov_time_add(hget, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done\n"); fflush(stdout);

	printf("getting w/ index..."); fflush(stdout);
	iget = cov_time_zero();
	for (j = 1; j < loops; j++) 
    for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		index_retrieve_first(pi, cols[i]);
		iget = cov_time_add(iget, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done\n"); fflush(stdout);	

	for (i = 0; i < n; i++) mem_free(cols[i]);

	/* freeing: */
	afree = cov_time_get();
	atree_destroy(pa);
	afree = cov_time_sub(cov_time_get(), afree);

	a2free = cov_time_get();
	atr_destroy(pa2);
	a2free = cov_time_sub(cov_time_get(), a2free);

	pafree = cov_time_get();
	patr_destroy(ppa);
	pafree = cov_time_sub(cov_time_get(), pafree);

	/* btree_print_raw(pb, rgb_print); */
	/* btree_print_sorted(pb, rgb_print); */

	bfree = cov_time_get();
	btree_destroy(pb);
	bfree = cov_time_sub(cov_time_get(), bfree);

	/* hash_dump(ph); */
	hfree = cov_time_get();
	hash_destroy(ph);
	hfree = cov_time_sub(cov_time_get(), hfree);

	/* index_dump(ph); */
	ifree = cov_time_get();
	index_destroy(pi);
	ifree = cov_time_sub(cov_time_get(), ifree);

	storage_destroy(ps);

	{ 
		char *b11, *b12, *b13, *b14;
		char *b21, *b22, *b23, *b24;
		char *b31, *b32, *b33, *b34;
		char *b41, *b42, *b43, *b44;
		char *b51, *b52, *b53, *b54;
		char *b61, *b62, *b63, *b64;

		printf("Perfs comparisons for %d find loops:\n\n\
      | create    | store     | find      | destroy   |\n\
------+-----------+-----------+-----------+-----------|\n\
atree | % 9s | % 9s | % 9s | % 9s |\n\
------+-----------+-----------+-----------+-----------|\n\
atr   | % 9s | % 9s | % 9s | % 9s |\n\
------+-----------+-----------+-----------+-----------|\n\
patr  | % 9s | % 9s | % 9s | % 9s |\n\
------+-----------+-----------+-----------+-----------|\n\
btree | % 9s | % 9s | % 9s | % 9s |\n\
------+-----------+-----------+-----------+-----------|\n\
hash  | % 9s | % 9s | % 9s | % 9s |\n\
------+-----------+-----------+-----------+-----------|\n\
index | % 9s | % 9s | % 9s | % 9s |\n\
------------------------------------------------------\n", 
			   loops,
			   b11 = cov_time_fmt(acreate), 
			   b12 = cov_time_fmt(astore), 
			   b13 = cov_time_fmt(aget), 
			   b14 = cov_time_fmt(afree), 

			   b21 = cov_time_fmt(a2create), 
			   b22 = cov_time_fmt(a2store), 
			   b23 = cov_time_fmt(a2get), 
			   b24 = cov_time_fmt(a2free), 

			   b31 = cov_time_fmt(pacreate), 
			   b32 = cov_time_fmt(pastore), 
			   b33 = cov_time_fmt(paget), 
			   b34 = cov_time_fmt(pafree), 

			   b41 = cov_time_fmt(bcreate), 
			   b42 = cov_time_fmt(bstore), 
			   b43 = cov_time_fmt(bget), 
			   b44 = cov_time_fmt(bfree), 

			   b51 = cov_time_fmt(hcreate), 
			   b52 = cov_time_fmt(hstore), 
			   b53 = cov_time_fmt(hget), 
			   b54 = cov_time_fmt(hfree),

			   b61 = cov_time_fmt(icreate), 
			   b62 = cov_time_fmt(istore), 
			   b63 = cov_time_fmt(iget), 
			   b64 = cov_time_fmt(ifree));

		free(b11); free(b12); free(b13); free(b14);
		free(b21); free(b22); free(b23); free(b24);
		free(b31); free(b32); free(b33); free(b34);
		free(b41); free(b42); free(b43); free(b44);
		free(b51); free(b52); free(b53); free(b54);
		free(b61); free(b62); free(b63); free(b64);
	}
	return 0;
}
