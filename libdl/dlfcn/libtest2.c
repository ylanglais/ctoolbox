static int static_int = 55;
       int extern_int = 66;

static void static_function() {
    extern_int--;
}

    void extern_function() {
    extern_int++;
}
    int  static_int_get() {
	return static_int;
}
    int  static_int_set(int i) {
	return static_int = i;
}
    void static_int_print() {
	printf("static_int = %d\n", static_int);
}
