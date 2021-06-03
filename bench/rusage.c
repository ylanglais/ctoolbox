#include <stdlib.h>
#include <stdio.h>
#include <sys/resource.h>
#include <time.h>

#include <tdb/tstmr.h>

int main(int n, char *a[]) {
	struct rusage ru;
	int pid;
	char b[100];	

	if (n != 2) return 1;

	pid = atoi(a[1]);

	if (getrusage(pid, &ru)) {
		fprintf(stderr, "%s: cannot get ressources usage info for pid %d\n", a[0], pid);
		return 2;
	}

	printf("Resource usage for process %d:\n", pid);
	printf("user time used :                         %s\n", tstamp_duration_fmt(b, ru.ru_utime));
	printf("system time used:                        %s\n", tstamp_duration_fmt(b, ru.ru_stime));
	printf("max resident set size (rss):             %10ld\n", ru.ru_maxrss);
	printf("integral resident set size (irss):       %10ld\n", ru.ru_idrss);
	printf("page faults not requiring physical I/O:  %10ld\n", ru.ru_minflt);
	printf("page faults requiring physical I/O:      %10ld\n", ru.ru_majflt);
	printf("swaps:                                   %10ld\n", ru.ru_nswap);	
	printf("block input operations:                  %10ld\n", ru.ru_inblock);
	printf("block output operations:                 %10ld\n", ru.ru_oublock);
	printf("messages sent:                           %10ld\n", ru.ru_msgsnd);
	printf("messages received:                       %10ld\n", ru.ru_msgrcv);
	printf("signals received:                        %10ld\n", ru.ru_nsignals);
	printf("voluntary context switches:              %10ld\n", ru.ru_nvcsw);
	printf("involuntary context switches:            %10ld\n", ru.ru_nivcsw);
	
	return 0;
}	
