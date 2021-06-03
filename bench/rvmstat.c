#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
extern int errno;

#include <tdb/err.h>

typedef struct {
	int r; /* in_queue for running */
	int b; /* blocked for resources I/O, paging, and so forth */
	int w; /* swapped */
} proc_t;

typedef struct {
	int swap; /* amount of swap space currently available (Kbytes=) */
	int free; /* size of the free list (Kbytes) */
} mem_t;

typedef struct {
	int re; /* page reclaims */
	int mf; /* minor fault   */ 
	int pi; /* kilobytes paged in */
	int po; /* kilobytes paged out */
	int	fr; /* kilobytes freed */
	int de; /* anticipated short-term memory shortfall (Kbytes) */
	int sr; /* pages scanned by clock algorithm */
} page_t;

typedef struct {
	int d0; 
	int d1;
	int d2;
	int d3;
} disk_t;

typedef struct {
	int in; /* (non clock) device interrupts */
	int sy; /* system calls */
	int cs; /* CPU context switches */
} faults_t;

typedef struct {
	int us; /* user time */
	int sy; /* system time */
	int id; /* idle time */
} cpu_t;

typedef struct {
	proc_t proc; 
	mem_t  mem;
	page_t page;
	disk_t disk;
	faults_t faults;
	cpu_t  cpu;
} vmstat_t, *pvmstat_t;

typedef int vmstat_a[20];

pvmstat_t
vmstat_new() {
	return (pvmstat_t) malloc(sizeof(vmstat_t));
}

pvmstat_t
vmstat_destroy(pvmstat_t v) {
	if (v) free(v);
	return NULL;
}

pvmstat_t
vmstat_get(pvmstat_t p) {
	FILE *f;
	char b[1000];
	int r;
	
	if (!p) if (!(p = vmstat_new())) return  NULL;

	if (!(f = popen("vmstat", "r"))) {
		err_error("cannot open/exec vmstat (%s)", strerror(errno));
		return vmstat_destroy(p);
	}

	r = fscanf(f, "%[^\n]\n", b);
	printf("r = %d, %s\n", r, b);
	r = fscanf(f, "%[^\n]\n", b);
	printf("r = %d, %s\n", r, b);
	r = fscanf(f, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
	&p->proc.r, &p->proc.b, &p->proc.w, 
	&p->mem.swap, &p->mem.free,
	&p->page.re, &p->page.mf, &p->page.pi, &p->page.po, &p->page.fr, &p->page.de, &p->page.sr, 
	&p->disk.d0, &p->disk.d1, &p->disk.d2, &p->disk.d3,
	&p->faults.in, &p->faults.sy, &p->faults.cs,
	&p->cpu.us, &p->cpu.sy, &p->cpu.id);

	printf("r = %d\n", r);

	fclose(f);
	return p;
}

void
vmstat_dump(pvmstat_t p) {	
	if (!p) {
		printf("no vmstat data.\n");
		return;
	}
	printf("vmstat : \n");
	printf("proc   / in_queue for running:                             %8d\n", p->proc.r);
	printf("proc   / blocked for resources I/O, paging, and so forth:  %8d\n", p->proc.b); 
	printf("proc   / swapped:                                          %8d\n", p->proc.w);
	printf("mem    / mount of swap space currently available (Kbytes): %8d\n", p->mem.swap);
	printf("mem    / size of the free list (Kbytes):                   %8d\n", p->mem.free);
	printf("page   / page reclaims:                                    %8d\n", p->page.re);
	printf("page   / minor fault:                                      %8d\n", p->page.mf);
	printf("page   / kilobytes paged in:                               %8d\n", p->page.pi);
	printf("page   / kilobytes paged out:                              %8d\n", p->page.po);
	printf("page   / kilobytes freed:                                  %8d\n", p->page.fr);
	printf("page   / anticipated short-term memory shortfall (Kbytes): %8d\n", p->page.de);
	printf("page   / pages scanned by clock algorithm:                 %8d\n", p->page.sr);
	printf("disk   / 0:                                                %8d\n", p->disk.d0);
	printf("disk   / 1:                                                %8d\n", p->disk.d1);
	printf("disk   / 2:                                                %8d\n", p->disk.d2);
	printf("disk   / 3:                                                %8d\n", p->disk.d3);
	printf("faults / (non clock) device interrupts:                    %8d\n", p->faults.in);
	printf("faults / system calls:                                     %8d\n", p->faults.sy);
	printf("faults / CPU context switches:                             %8d\n", p->faults.cs);	
	printf("cpu    / user time:                                        %8d\n", p->cpu.us);
	printf("cpu    / sy:                                               %8d\n", p->cpu.sy);
	printf("cpu    / id:                                               %8d\n", p->cpu.id);
}


#ifdef _std_alone_ 
int 
main(void) {
	pvmstat_t v;
	v = vmstat_new();
	vmstat_get(v);
	vmstat_dump(v);
	vmstat_destroy(v);
	return 0;
}
#endif
