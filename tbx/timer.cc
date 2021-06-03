
#include <timer.hh>
#include <stdio.h>

void
Timer::reset() {
	State = ZERO;
	Start.set(0);
	Elapsed.set(0);
}

Timer::Timer() {
	reset();
}

int
Timer::start() {
	if (State == PAUSED || State == ZERO) {
		State =  RUNNING;
		Start.now();
		return 0;
	}
	return ERROR;
}

int 
Timer::stop() {
	if (State == RUNNING || State == PAUSED) {
		Timestamp ts;
		ts.now();
		Elapsed = (ts - Start);
		Start.set(0);
		State = STOPPED;
	}
	return 0;
}

Timestamp
Timer::pause() {
	
	if (State == RUNNING) {
		Timestamp ts;
		ts.now();
		Elapsed += ts - Start;
		Start.set(0);
		State = PAUSED;
	}
	return Elapsed;
}

Timestamp
Timer::laps() {
	Timestamp ts;
	ts.now();
	Elapsed += ts - Start;
	Start = ts;
	return Elapsed;
}

Timestamp
Timer::elapsed() {
	return Elapsed;
}	

void
Timer::dump() {
	char b[64];
	
	switch (State) {
	case UNKOWN:
		printf("State  : UNKOWN\n");
		break;
	case ZERO:
		printf("State  : ZERO\n");
		break;
	case RUNNING:
		printf("State  : RUNNING\n");
		break;
	case PAUSED:
		printf("State  : PAUSED\n");
		break;
	case STOPPED:
		printf("State  : STOPPED\n");
		break;
	case ERROR:
		printf("State  : ERROR\n");
		break;
	case BADTIMER:
		printf("State  : BADTIMER\n");
		break;
	default:
		printf("invalid State (%d)\n", State);
	}
	printf("start  : %s\n", Start.fmt(b));
	printf("elapsed: %s\n", Elapsed.duration_fmt(b));
} 

