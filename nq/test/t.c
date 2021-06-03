#include <stdlib.h>
#include <stdio.h>
#include <tbx/futl.h>
#include <tbx/tstmr.h>

int main(void) {
	pfutl_file_info_t dir;
	int   count;

	dir = futl_dir("./", "[0-9]*\\.[0-9][0-9][0-9]\\.[0-9]*[0-9]", futl_SORT_CTIME, futl_ORDER_ASC, &count);

	int i;
	for (i = 0; i < count; i++) {
		
		printf("--> file %d\n", i);		
		
		char b[100];
		printf("File %d:\n", i);
		printf("\tname: %s\n", dir[i].fname);
		printf("\tname: %lo\n", dir[i].size);
		tstamp_t s;
		s.tv_sec  = dir[i].ctime.tv_sec;
		s.tv_usec = dir[i].ctime.tv_nsec / 1000;
		printf("\tcreation: %s\n", tstamp_fmt(b, s));
	}
return 0;
}


