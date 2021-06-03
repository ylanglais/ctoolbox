#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

//#include "storage.h"
#include "coverage.h"
#include "hash.h"
#include "index.h"
#include "atree.h"
#include "atr.h"
#include "patr.h"
//#include "btree.h"
#include "mem.h"
#include "tstmr.h"

#define WORDFILE "ods5.txt"
#define NAMESIZE 16
#define loops   10

#define tstruct struct timeval


typedef struct {
	char s[16];
} word_t, *pword_t;


int cmp(void *a, void *b) {
	if (a && b) 
		return strcmp(((pword_t) a)->s, ((pword_t) b)->s);
	return 0;
}

void word_print(void *w) {
	if (w) printf("%s\n", ((pword_t) w)->s);
}

int main(int na, char **a) {
	int i, j, n;
	//pstorage_t ps;
	phash_t  ph;
	pindex_t pi;
	patree_t pa;
	patr_t   pa2;
	ppatr_t  ppa;
	//pbtree_t pb;
	FILE *f;
	word_t cols[378988];
	struct timeval  acreate, a2create, pacreate, hcreate, icreate, astore, a2store, pastore, hstore, istore,
					aget, a2get, paget, hget, iget, afree, a2free, pafree, hfree, ifree, tmp;

	/*if (!(ps = storage_new(sizeof(word_t), 100))) exit(1); */

	acreate   = cov_time_get();
	if (!(pa  = atree_new())) exit(2);
	acreate   = cov_time_sub(cov_time_get(), acreate);

	a2create  = cov_time_get();
	if (!(pa2 = atr_new())) exit(2);
	a2create  = cov_time_sub(cov_time_get(), a2create);

	pacreate  = cov_time_get();
	if (!(ppa = patr_new("ABCDEFGHIJKLMNOPQRSTUVWXYZ"))) exit(2);
	pacreate  = cov_time_sub(cov_time_get(), pacreate);

	hcreate   = cov_time_get();
	if (!(ph  = hash_new_sized(200000))) exit(3);
	hcreate   = cov_time_sub(cov_time_get(), hcreate);

	icreate   = cov_time_get();
	if (!(pi  = index_new())) exit(3);
	icreate   = cov_time_sub(cov_time_get(), icreate);
	

	/* storing */
	if (!(f = fopen(WORDFILE, "r"))) {
		printf("cannot open \"%s\" file, exiting\n", WORDFILE);
		//storage_destroy(ps);
		atree_destroy(pa);
		atr_destroy(pa2);
		patr_destroy(ppa);
		//btree_destroy(pb);
		hash_destroy(ph);
		index_destroy(pi);
		exit(1);
	}
	printf("read word data from %s\n", WORDFILE);

	astore = a2store = pastore = hstore = istore = cov_time_zero();
	i = 0;
	while (!feof(f)) {
		void *p;
		char b[200];

		if (fscanf(f, "%[^\n]\n", cols[i].s) == 1) { 
			p = &cols[i].s;

			tmp = cov_time_get();
			atree_store(pa, cols[i].s, (char *) p); 
			astore = cov_time_add(astore, cov_time_sub(cov_time_get(), tmp));

			tmp = cov_time_get();
			atr_store(pa2, cols[i].s, (char *) p); 
			a2store = cov_time_add(a2store, cov_time_sub(cov_time_get(), tmp));

			tmp = cov_time_get();
			patr_store(ppa, cols[i].s, (char *) p); 
			pastore = cov_time_add(pastore, cov_time_sub(cov_time_get(), tmp));


			//tmp = cov_time_get();
			//btree_store(pb, (void *) p); 
	//		bstore = cov_time_add(bstore, cov_time_sub(cov_time_get(), tmp));

			tmp = cov_time_get();
			hash_insert(ph, cols[i].s, (char *) p);
			hstore = cov_time_add(hstore, cov_time_sub(cov_time_get(), tmp));

			tmp = cov_time_get();
			index_add(pi, cols[i].s, (char *) p);
			istore = cov_time_add(istore, cov_time_sub(cov_time_get(), tmp));

			i++; 
			if (!(i % 10000)) { 
				printf("%s %d\n", tstamp_fmt(b, tstamp_get()), i); fflush(stdout); 
			}
		}
	}
	fclose(f);
	printf("\n");
	n = i; 

	/* btree_print_raw(pb, rgb_print); */
	/* btree_print_sorted(pb, rgb_print); */

	/* getting: */
	printf("getting w/ atree %d x %d times...", loops, n); fflush(stdout);
	aget = cov_time_zero();
	for (j = 1; j < loops; j++) 
    for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		atree_retrieve(pa, cols[i].s);
		aget = cov_time_add(aget, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done\n"); fflush(stdout);
	
	printf("getting w/ atr %d x %d times...", loops, n); fflush(stdout);
	a2get = cov_time_zero();
	for (j = 1; j < loops; j++) 
    for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		atr_retrieve(pa2, cols[i].s);
		a2get = cov_time_add(a2get, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done\n"); fflush(stdout);

	printf("getting w/ patr %d x %d times...", loops, n); fflush(stdout);
	paget = cov_time_zero();
	for (j = 1; j < loops; j++) 
    for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		patr_retrieve(ppa, cols[i].s);
		paget = cov_time_add(paget, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done\n"); fflush(stdout);

	printf("getting w/ hash %d x %d times...", loops, n); fflush(stdout);
	hget = cov_time_zero();
	for (j = 1; j < loops; j++) 
    for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		hash_retrieve(ph, cols[i].s);
		hget = cov_time_add(hget, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done\n"); fflush(stdout);

	printf("getting w/ index %d x %d times...", loops, n); fflush(stdout);
	iget = cov_time_zero();
	for (j = 1; j < loops; j++) 
    for (i = 0; i < n; i++) {
		tmp = cov_time_get();
		index_retrieve_first(pi, cols[i].s);
		iget = cov_time_add(iget, cov_time_sub(cov_time_get(), tmp));
	}
	printf("done\n"); fflush(stdout);	

	//for (i = 0; i < n; i++) mem_free(cols[i].s);

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

	/* hash_dump(ph); */
	hfree = cov_time_get();
	hash_destroy(ph);
	hfree = cov_time_sub(cov_time_get(), hfree);
	/* index_dump(ph); */
	ifree = cov_time_get();
	index_destroy(pi);
	ifree = cov_time_sub(cov_time_get(), ifree);

	//storage_destroy(ps);

	{ 
		char *b11, *b12, *b13, *b14;
		char *b21, *b22, *b23, *b24;
		char *b31, *b32, *b33, *b34;
		char *b41, *b42, *b43, *b44;
		char *b51, *b52, *b53, *b54;

		printf("Perfs comparisons :\n\n\
      | create    | store     | find      | destroy   |\n\
------+-----------+-----------+-----------+-----------|\n\
atree | % 9s | % 9s | % 9s | % 9s |\n\
------+-----------+-----------+-----------+-----------|\n\
atr   | % 9s | % 9s | % 9s | % 9s |\n\
------+-----------+-----------+-----------+-----------|\n\
patr  | % 9s | % 9s | % 9s | % 9s |\n\
------+-----------+-----------+-----------+-----------|\n\
hash  | % 9s | % 9s | % 9s | % 9s |\n\
------+-----------+-----------+-----------+-----------|\n\
index | % 9s | % 9s | % 9s | % 9s |\n\
------------------------------------------------------\n", 
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

			   b41 = cov_time_fmt(hcreate), 
			   b42 = cov_time_fmt(hstore), 
			   b43 = cov_time_fmt(hget), 
			   b44 = cov_time_fmt(hfree),

			   b51 = cov_time_fmt(icreate), 
			   b52 = cov_time_fmt(istore), 
			   b53 = cov_time_fmt(iget), 
			   b54 = cov_time_fmt(ifree));

		free(b11); free(b12); free(b13); free(b14);
		free(b21); free(b22); free(b23); free(b24);
		free(b31); free(b32); free(b33); free(b34);
		free(b41); free(b42); free(b43); free(b44);
		free(b51); free(b52); free(b53); free(b54);
	}
	return 0;
}
