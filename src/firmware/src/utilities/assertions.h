#ifndef BUFF_LOGGERS
#define BUFF_LOGGERS
/*
	Plans for this is to become like unity macros
	- used for assertions and debug printing
	- need to support unit testing
	- if it gets big enough start making a fancy serial layer (PC side)
*/

#define TEST_INFO_SCALAR(msg, a) { \
	Serial.print("[TEST INFO]\t"); \
	Serial.print(msg); \
	Serial.printf(": %f\n", a); \
}

#define TEST_INFO(msg, op, a, b) { \
	Serial.print("[TEST INFO]\t"); \
	Serial.print(msg); \
	Serial.printf(": %f %s %f\n", a, op, b); \
}

template <typename T> int assert_eq(T a, T b, String message) {
	if (abs(a - b) > 1E-15) {
		TEST_INFO(message, "!=", a, b);
		return 1;
	}
	return 0;
}

template <typename T> int assert_eq(T* a, T b, String message, int n) {
	int errors = 0;
	for (int i = 0; i < n; i++) {
		errors += assert_eq(a[i], b, message + " [" + String(i) + "]");
	}
	return errors;
}

template <typename T> int assert_eq(T* a, T* b, String message, int n) {
	int errors = 0;
	for (int i = 0; i < n; i++) {
		errors += assert_eq(a[i], b[i], message + " [" + String(i) + "]");
	}
	return errors;
}

template <typename T> int assert_neq(T a, T b, String message) {
	if (a == b) {
		TEST_INFO(message, "==", a, b);
		return 1;
	}
	return 0;
}

template <typename T> int assert_neq(T* a, T b, String message, int n) {
	int errors = 0;
	for (int i = 0; i < n; i++) {
		errors += assert_neq(a[i], b, message + " [" + String(i) + "]");
	}
	return errors;
}

template <typename T> int assert_neq(T* a, T* b, String message, int n) {
	int errors = 0;
	for (int i = 0; i < n; i++) {
		errors += assert_eq(a[i], b[i], message + " [" + String(i) + "]");
	}
	return errors;
}

template <typename T> int assert_leq(T a, T b, String message) {
	if (a > b) {
		TEST_INFO(message, "!<=", a, b);
		return 1;
	}
	return 0;
}

template <typename T> int assert_leq(T* a, T b, String message, int n) {
	int errors = 0;
	for (int i = 0; i < n; i++) {
		errors += assert_leq(a[i], b, message + " [" + String(i) + "]");
	}
	return errors;
}

template <typename T> int assert_leq(T* a, T* b, String message, int n) {
	int errors = 0;
	for (int i = 0; i < n; i++) {
		errors += assert_leq(a[i], b[i], message + " [" + String(i) + "]");
	}
	return errors;
}

template <typename T> int assert_geq(T a, T b, String message) {
	if (a < b) {
		TEST_INFO(message, "!>=", a, b);
		return 1;
	}
	return 0;
}

template <typename T> int assert_geq(T* a, T b, String message, int n) {
	int errors = 0;
	for (int i = 0; i < n; i++) {
		errors += assert_geq(a[i], b, message + " [" + String(i) + "]");
	}
	return errors;
}

template <typename T> int assert_geq(T* a, T* b, String message, int n) {
	int errors = 0;
	for (int i = 0; i < n; i++) {
		errors += assert_geq(a[i], b[i], message + " [" + String(i) + "]");
	}
	return errors;
}

int assert_not_nan(float a){
	if (isnan(a)) {
		TEST_INFO_SCALAR("Value is NaN", a);
		return 1;
	}
	return 0;
}

template <typename T> int assert_not_nan(T* a, String message, int n) {
	int errors = 0;
	for (int i = 0; i < n; i++) {
		errors += assert_not_nan(a[i], message + " [" + String(i) + "]");
	}
	return errors;
}

void timer_print(uint32_t time, String message) {
	Serial.print("[TIMER]\t");
		Serial.print(message);
		Serial.printf("\t%i\n", time);
}

#endif