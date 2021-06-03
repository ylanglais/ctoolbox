#ifndef _Timer_h_ 
#define _Timer_h_ 

#include <timestamp.hh>

class Timer {
private:
	typedef enum { UNKOWN, ZERO, RUNNING, PAUSED, STOPPED, ERROR, BADTIMER } state_t;
	state_t   State;
	Timestamp Start;
	Timestamp Elapsed;

public:
	Timer();
	
	void reset();
	int start();
	int stop();
	Timestamp pause();
	Timestamp elapsed();
	Timestamp laps();
	void dump();
};

#endif
