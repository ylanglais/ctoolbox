#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <timestamp.hh>

int main(void) {
	char b[100], b1[100], b2[100];
	
	Timestamp t, t1, t2, r;

	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));

	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));


	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));


	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));
	t.now(); printf("%s\n", t.fmt(b));

	t1.sec(12);
	t2.sec(13);
	
	r = t2 - t1;
	printf("%s - %s = %s\n", t2.duration_fmt(b), t1.duration_fmt(b1), r.duration_fmt(b2));
	r = t2 + t1;
	printf("%s + %s = %s\n", t2.duration_fmt(b), t1.duration_fmt(b1), r.duration_fmt(b2));
	
	t1 *= 60;
	t2 *= 60;
	
	r = t2 - t1;
	printf("%s - %s = %s\n", t2.duration_fmt(b), t1.duration_fmt(b1), r.duration_fmt(b2));
	r = t2 + t1;
	printf("%s + %s = %s\n", t2.duration_fmt(b), t1.duration_fmt(b1), r.duration_fmt(b2));

	t1 *= 60;
	t2 *= 60;

	r = t2 - t1;
	printf("%s - %s = %s\n", t2.duration_fmt(b), t1.duration_fmt(b1), r.duration_fmt(b2));
	r = t2 + t1;
	printf("%s + %s = %s\n", t2.duration_fmt(b), t1.duration_fmt(b1), r.duration_fmt(b2));

	t1.sec(1);

	r = t1 * 2;
	printf("%s * %d = %s\n", t1.duration_fmt(b), 2, r.duration_fmt(b1));
	
	r = t1 / 4;
	printf("%s / %d = %s\n", t1.duration_fmt(b), 4, r.duration_fmt(b1));

	t.now();
	printf("elapsed since 1/1/1970 = %s\n", t.duration_fmt(b));

	return 0;
}
