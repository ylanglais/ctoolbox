
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include <tbx/tstmr.h>



int main(int n, char *a[]) {
	int r, IN;
	char b[25];
	fd_set readfds;
	tstamp_t start;
	struct termios term, saved;
	struct timeval ts;

	ts.tv_sec  =  0;
	ts.tv_usec = 10000;

	IN = fileno(stdin);

	/* remove line buffering : */
	tcgetattr(IN, &term),
	saved = term;
	term.c_lflag &= ~(ICANON|ECHO|ECHONL|IEXTEN);
	/* default line buffer = 4 on several Unix => make it 1 */
	term.c_cc[VTIME] = 0;
	term.c_cc[VMIN]  = 1;

	tcsetattr(IN, TCSANOW, &term);

	setbuf(stdin, NULL);

	FD_ZERO(&readfds);
	FD_SET(IN, &readfds);

	start = tstamp_get();

	printf("%s\b ", tstamp_duration_fmt(b, tstamp_sub(tstamp_get(), start = tstamp_get()))); fflush(stdout);

	while (((r = select(IN + 1, &readfds, NULL, NULL, &ts)) <= 0) && !(FD_ISSET(IN, &readfds))) {
		FD_SET(IN, &readfds);
		printf("\b\b\b\b\b\b\b\b\b\b\b\b%s\b ", tstamp_duration_fmt(b, tstamp_sub(tstamp_get(), start))); fflush(stdout); 
	}
	getchar();
	setbuf(stdin, NULL);

	tcsetattr(IN, TCSANOW, &saved);

	printf("\n");
	return 0;
}
