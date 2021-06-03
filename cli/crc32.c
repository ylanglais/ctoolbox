#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <tbx/crc_32.h>

extern int errno;

int
main(int n, char *a[]) {
	unsigned char *p;
	int i, fd;
	unsigned int cksum;

	if (n < 2) {
		printf("no file to check\n");
		return 1;
	}

	for (i = 1; i < n; i++) {
		struct stat st;
		if (stat(a[i], &st)) {
			fprintf(stderr, "cannot stat %f: %s\n", a[i], strerror(errno));
			continue;
		} 
		if (!(p = malloc(st.st_size))) {
			fprintf(stderr, "cannot reserve %u bytes for file %s\n", st.st_size, a[i]);
			continue;
		}
		if ((fd = open(a[i], O_RDONLY)) < 1) {
			fprintf(stderr, "cannot open file %s: %s\n", a[i], strerror(errno));
			free(p);
			continue;
		}		
		if ((read(fd, p, st.st_size) < st.st_size)) {
			fprintf(stderr, "cannot read all %sfile...\n", a[i]);
			close(fd);
			free(p);
			continue;
		} 
		
		cksum = crc_32(p, st.st_size);
		printf("%u %d %s\n", cksum, st.st_size, a[i]);

		close(fd);
		free(p);
	}

	return 0;
}

