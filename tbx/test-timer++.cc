
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <timer.hh>

int main(void) {
	char b[64];
	Timer t;

	printf("start timer\n");
	t.start();

	printf("sleep(2)\n");
	sleep(2);

	t.pause();
	printf("elapsed = %s\n", t.elapsed().duration_fmt(b));

	t.reset();
	
	printf("restart timer\n");
	t.start();
	printf("sleep(10)\n");
	sleep(10);
	
	t.pause();
	printf("elapsed = %s\n", t.elapsed().duration_fmt(b));
}
