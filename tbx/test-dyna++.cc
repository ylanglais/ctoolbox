#include <stdlib.h>
#include <stdio.h>
#include <dyna.hh>

typedef void (*print_cb)(char *);

void
dyna_dump_idx_seq(Dyna *da, print_cb cb) {
	int i;
	for (i = 0; i < da->used(); i++) {
		printf("%4d: ", i);
		cb(da->get(i));
		printf("\n");
	}
} 

void
dyna_dump_ptr_seq(Dyna *da, print_cb cb) {
	int ip, ig;
	for (ip = 0; ip < da->pages(); ip++) for (ig = 0; ig < da->grain(); ig++) {
		printf("page %2d offset %2d: ", ip, ig);
		cb((char *) da->data_ptr(ip) + ig * da->unit());
		printf("\n");
	}
}

void
print_int(char *p) {
	int *pi;
	pi = (int *) p;
	printf("%d", *pi);
}

#define printadd(da, action, val) {printf("da->%s %d\n", #action, val); da->action((char *) &val); } 
#define printdel(da, action, index) {printf("da->%s %d\n", #action, index); da->action(index); }
#define printins(da, action, index, val) {printf("da->%s %d at %d\n", #action, val, index); da->action(index, (char *) &val); } 
	
int
main() {
	int i;
	Dyna *da;
	
	if (!(da = new Dyna(sizeof(int), 3))) return 1;
	
	i = 1; printadd(da, add, i);
	i = 2; printadd(da, add, i);
	i = 3; printadd(da, add, i);
	i = 4; printadd(da, add, i);
	i = 5; printadd(da, add, i);
	i = 6; printadd(da, add, i);
	i = 7; printadd(da, add, i);
	i = 8; printadd(da, add, i);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	printdel(da, remove, 3);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);

	printdel(da, remove, 6);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	i = 9;  printadd(da, add,  i);
	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);

	i = 10; printins(da, insert, 5, i);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	i = 11; printins(da, insert, 1, i);
	i = 12; printins(da, insert, 1, i);
	i = 13; printins(da, insert, 1, i);
	i = 14; printins(da, insert, 1, i);
	i = 15; printins(da, insert, 1, i);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);

	i = 16; printins(da, insert, 0, i);
	i = 17; printins(da, insert, 0, i);
	i = 18; printins(da, insert, 0, i);
	i = 19; printins(da, insert, 0, i);
	i = 20; printins(da, insert, 0, i);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);

	i = 7; printins(da, set, 15, i);
	i = 8; printins(da, set, 16, i);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	printdel(da, remove, 0);
	printdel(da, remove, 0);
	printdel(da, remove, 0);
	printdel(da, remove, 0);
	printdel(da, remove, 0);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	printdel(da, remove, 1);
	printdel(da, remove, 1);
	printdel(da, remove, 1);
	printdel(da, remove, 1);
	printdel(da, remove, 1);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	i = 10; printadd(da, add, i);
	i = 11; printadd(da, add, i);
	i = 12; printadd(da, add, i);
	i = 13; printadd(da, add, i);
	i = 14; printadd(da, add, i);
	i = 15; printadd(da, add, i);
	i = 16; printadd(da, add, i);
	i = 17; printadd(da, add, i);
	i = 18; printadd(da, add, i);
	i = 19; printadd(da, add, i);
	i = 20; printadd(da, add, i);

	i = 4; printins(da, insert, 3, i);

	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);
	
	da->remove(da->get(5)); 
	
	dyna_dump_idx_seq(da, print_int);
	dyna_dump_ptr_seq(da, print_int);

	return 0;	
}


