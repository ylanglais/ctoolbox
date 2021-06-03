int _incr(int a) {
    return a+1;
}

int
main(void) {
	int a = 1;
	int _incr(int a) {
		return a+1;
	}
	_incr(a);
	_incr(a);
	_incr(a);
	_incr(a);
	return 0;
}

