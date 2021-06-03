#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <procfs.h>
#include <unistd.h>

int
main(int argc, char *argv[]) {
    int psp;
    struct psinfo ps;
    char fn[100];


    /* One argument - the PID to be examined */ 
	if (argc != 2) {
        printf("Usage: program procid\n");
        return 1;
    }

    /* Build a string to the psinfo file in procfs.i.e. / proc / <pid > /psinfo */ 

	sprintf(fn, "/proc/%s/psinfo", argv[1]);

    /* Open the psinfo file and read the psinfo structure */ 
	if ((psp = open(fn, O_RDONLY)) == -1) {
        perror("ps open");
        return 2;
    }
    if (read(psp, &ps, sizeof(struct psinfo)) == -1) {
        perror("read");
        return 3;
    }

    close(psp);

	printf("name of exec'ed file:                   %s\n",   ps.pr_fname);
	printf("%% of recent cpu time used by all lwps: %12.2f\n", (float) ps.pr_pctcpu * 100. / (float) 0x8000);
	printf("%% of system memory used by proces:     %12.2f\n", (float) ps.pr_pctmem * 100. / (float) 0x8000);
	printf("size of process image in Kbytes:        %8d\n", ps.pr_size);
	printf("resident set size in Kbytes:            %8d\n", ps.pr_rssize);

	return 0;
}
