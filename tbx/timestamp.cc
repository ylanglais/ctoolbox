
#include <stdlib.h>
#include <stdio.h>
#include <timestamp.hh>

Timestamp::Timestamp() { 
	tstamp.tv_sec = tstamp.tv_usec = 0; 
}

Timestamp::Timestamp(long sec, long usec) { 
	tstamp.tv_sec = sec, tstamp.tv_usec = usec; 
}

Timestamp::Timestamp(struct timeval t) {
	tstamp.tv_sec = t.tv_sec, tstamp.tv_usec = t.tv_usec; 
}

//Timestamp::Timestamp(Timestamp t) { 
//	tstamp.tv_sec = t.tstamp.tv_sec, tstamp.tv_usec = t.tstamp.tv_usec; 
//}

void Timestamp::set(long sec, long usec)  {  
	tstamp.tv_sec = sec, tstamp.tv_usec = usec; 
}

void Timestamp::set(Timestamp t)  { 
	tstamp.tv_sec = t.tstamp.tv_sec, tstamp.tv_usec = t.tstamp.tv_usec; 
}
	
long  
Timestamp::sec()  { 
	return tstamp.tv_sec;
}

long  
Timestamp::sec(long sec)  { 
	return tstamp.tv_sec = sec;
}

long  
Timestamp::usec() { 
	return tstamp.tv_usec;
}

long  
Timestamp::usec(long usec) { 
	return tstamp.tv_usec = usec;
}

Timestamp Timestamp::now() { 
	struct timezone tz; 
	gettimeofday(&tstamp, &tz); 
	return *this; 
}

Timestamp Timestamp::operator=(Timestamp t) {
	tstamp.tv_sec  = t.tstamp.tv_sec;
	tstamp.tv_usec = t.tstamp.tv_usec;
	return *this;
}

Timestamp Timestamp::add(Timestamp t) {
	Timestamp r;

	r.tstamp.tv_sec  = tstamp.tv_sec  + t.tstamp.tv_sec;
	r.tstamp.tv_usec = tstamp.tv_usec + t.tstamp.tv_usec;
	if (r.tstamp.tv_usec >= 1000000) {
		r.tstamp.tv_usec -= 1000000; 
		r.tstamp.tv_sec  += 1;
	}
	return r;
}

Timestamp Timestamp::operator+(Timestamp t) {
	return add(t);
}

Timestamp Timestamp::operator+=(Timestamp t) {
	return (*this) = add(t);
}

Timestamp Timestamp::sub(Timestamp t) {
	Timestamp r; 
	r.tstamp.tv_sec  = tstamp.tv_sec  - t.tstamp.tv_sec;
	r.tstamp.tv_usec = tstamp.tv_usec - t.tstamp.tv_usec;
	if (r.tstamp.tv_usec < 0) {
		r.tstamp.tv_usec += 1000000; 
		r.tstamp.tv_sec -= 1;
	}
	return r;

}

Timestamp Timestamp::operator-(Timestamp t) { 
	return sub(t); 
}

Timestamp Timestamp::operator-=(Timestamp t) {
	return (*this) = sub(t);
}

Timestamp Timestamp::mul(long k) {
	Timestamp r;
	r.tstamp.tv_sec  = k * tstamp.tv_sec;
	r.tstamp.tv_usec = k * tstamp.tv_usec;

	if (r.tstamp.tv_usec >= 1000000) {
		r.tstamp.tv_sec += tstamp.tv_usec / 1000000;
		r.tstamp.tv_usec = tstamp.tv_usec % 1000000;
	}
	return r;
}

Timestamp Timestamp::operator*(long k) { 
	return mul(k); 
}

Timestamp Timestamp::operator*=(long k) {
	return (*this) = mul(k);
}

Timestamp Timestamp::div(long k) { 
	Timestamp r;
	r.tstamp.tv_sec   =  tstamp.tv_sec  / k;
	r.tstamp.tv_usec  =  tstamp.tv_usec / k;
	r.tstamp.tv_usec += (tstamp.tv_sec % k) * 1000000 / k;

	return r;
}

Timestamp Timestamp::operator/(long k) {
	return div(k); 
}

Timestamp Timestamp::operator/=(long k) {
	return (*this) = div(k); 
}

int Timestamp::cmp(Timestamp t) {
	if (tstamp.tv_sec  < t.tstamp.tv_sec)  return  1;
	if (tstamp.tv_sec  > t.tstamp.tv_sec)  return -1;
	if (tstamp.tv_usec < t.tstamp.tv_usec) return  1;
	if (tstamp.tv_usec > t.tstamp.tv_usec) return -1;
	return 0;
}

bool Timestamp::lt(Timestamp t) {
	if (cmp(t) < 0) return true;
	return false;
}

bool Timestamp::operator<(Timestamp t) {
	return lt(t);
}

bool Timestamp::le(Timestamp t) {
	if (cmp(t) > 0) return false;
	return true;	
}

bool Timestamp::operator<=(Timestamp t) {
	return le(t);
}

bool Timestamp::gt(Timestamp t) {
	if (cmp(t) > 0) return true;
	return false;
}

bool Timestamp::operator>(Timestamp t) {
	return gt(t);
}

bool Timestamp::ge(Timestamp t) {
	if (cmp(t) < 0) return false;
	return true;
}

bool Timestamp::operator>=(Timestamp t) {
	return ge(t);
}

bool Timestamp::eq(Timestamp t) {
	if (cmp(t) == 0) return true;
	return false;
}

bool Timestamp::operator==(Timestamp t) {
	return eq(t);
}

char *
Timestamp::fmt(char *buff25) {
	struct tm *t;
	t = localtime(&tstamp.tv_sec);
	sprintf(buff25, "%02d/%02d/%04d %02d:%02d:%02d.%03d", 
		t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec, (int) tstamp.tv_usec / 1000);
	return buff25;
}

char *
Timestamp::duration_fmt(char *buff25) {
	struct tm *t;
	t = localtime(&tstamp.tv_sec);

	if (t->tm_year > 70) 
		sprintf(buff25, "%d years %d months %d days %02d:%02d:%02d.%03d", 
		t->tm_year - 70, t->tm_mon, t->tm_mday - 1, t->tm_hour - 1, t->tm_min, t->tm_sec, (int) tstamp.tv_usec / 1000);
	else if (t->tm_mon > 0) 
		sprintf(buff25, "%d months %d days %02d:%02d:%02d.%03d", 
		t->tm_mon, t->tm_mday - 1, t->tm_hour - 1, t->tm_min, t->tm_sec, (int) tstamp.tv_usec / 1000);
	else if (t->tm_mday > 1) 
		sprintf(buff25, "%d days %02d:%02d:%02d.%03d", 
		t->tm_mday - 1, t->tm_hour - 1, t->tm_min, t->tm_sec, (int) tstamp.tv_usec / 1000);
	else 
		sprintf(buff25, "%02d:%02d:%02d.%03d", 
		t->tm_hour - 1, t->tm_min, t->tm_sec, (int) tstamp.tv_usec / 1000);
	
	return buff25;
}

