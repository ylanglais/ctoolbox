
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#include <tbx/tstmr.h>

static int            _ubterm_stdin_fd_;  
static struct termios _ubterm_ios_saved_;

#include "ubterm.h"

void ubterm_ini() {
	struct termios term;

	_ubterm_stdin_fd_ = fileno(stdin);

	/* Initialize term structure with current parameters: */
	tcgetattr(_ubterm_stdin_fd_, &term),

	/* Save current parameters: */ 
	_ubterm_ios_saved_ = term;

	/* Disable line buffering : */
	term.c_lflag &= ~(ICANON|ECHO|ECHONL|IEXTEN);

	/* Default line buffer = 4 on several Unix => make it 1 */
	term.c_cc[VTIME] = 0;
	term.c_cc[VMIN]  = 1;

	/* Set new term parameters: */
	tcsetattr(_ubterm_stdin_fd_, TCSANOW, &term);

	/* Clean line buffering: */
	setbuf(stdin, NULL);
	
	/* Make sure to call then ubterm_fini at exit: */
	atexit(ubterm_fini);
}

void ubterm_fini() {
	/* Clean line buffering: */
	setbuf(stdin, NULL);

	/* Restore initial parameters: */
	tcsetattr(_ubterm_stdin_fd_, TCSANOW, &_ubterm_ios_saved_);
}
