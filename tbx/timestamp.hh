
#ifndef _Timestamp_h_ 
#define _Timestamp_h_ 

#include <time.h>
#include <sys/time.h>

class Timestamp {
private:
	struct timeval tstamp;

	Timestamp add(Timestamp t);
	Timestamp sub(Timestamp t);
	Timestamp mul(long k);
	Timestamp div(long k);

	int cmp(Timestamp t);

	bool lt(Timestamp t);
	bool le(Timestamp t);
	bool gt(Timestamp t);
	bool ge(Timestamp t);
	bool eq(Timestamp t);

public:

	Timestamp();
	Timestamp(long sec, long usec = 0);
	Timestamp(struct timeval t);
//	Timestamp(Timestamp t);

	void set(long sec = 0, long usec = 0);
	void set(Timestamp t);
	
	long  sec();
	long  sec(long sec);
	long  usec();
	long  usec(long usec);

	Timestamp now();

	Timestamp operator = (Timestamp t);

	Timestamp operator +  (Timestamp t);
	Timestamp operator += (Timestamp t);
	Timestamp operator -  (Timestamp t);
	Timestamp operator -= (Timestamp t);
	Timestamp operator *  (long k);
	Timestamp operator *= (long k);
	Timestamp operator /  (long k);
	Timestamp operator /= (long k);
	
	
	bool operator <  (Timestamp t);
	bool operator <= (Timestamp t);
	bool operator >  (Timestamp t);
	bool operator >= (Timestamp t);
	bool operator == (Timestamp t);

	char *fmt(char *buff25);
	char *duration_fmt(char *buff25);
};

#endif
