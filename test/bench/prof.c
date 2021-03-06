/* radare - LGPL - Copyright 2009-2012 - pancake */

#define R_API
#define R_ABS(x) (((x)<0)?-(x):(x))

#ifdef _MSC_VER

#include "Windows.h"

typedef struct r_prof_t {
	LARGE_INTEGER begin;
	double result;
} RProfile;

R_API void r_prof_start(struct r_prof_t *p) {
	p->result = 0.0;
	QueryPerformanceCounter (&p->begin);
}

R_API double r_prof_end(struct r_prof_t *p) {
	LARGE_INTEGER end, freq, diff;
	QueryPerformanceCounter (&end);
	QueryPerformanceFrequency (&freq);
	diff.QuadPart = end.QuadPart - p->begin.QuadPart;
	p->result = R_ABS ((double)diff.QuadPart / freq.QuadPart);
	return R_ABS (diff.QuadPart < 0);
}

#else

#include <sys/time.h>

typedef struct r_prof_t {
        struct timeval begin;
        double result;
} RProfile;

typedef struct timeval tv;

// Subtract the 'tv' values begin from end, storing result in RESULT
// Return 1 if the difference is negative, otherwise 0.
static int timeval_subtract(tv *result, tv *end, tv *begin) {
	// Perform the carry for the later subtraction by updating Y
	if (end->tv_usec < begin->tv_usec) {
		int nsec = (begin->tv_usec - end->tv_usec) / 1000000 + 1;
		begin->tv_usec -= 1000000 * nsec;
		begin->tv_sec += nsec;
	}
	if (end->tv_usec - begin->tv_usec > 1000000) {
		int nsec = (end->tv_usec - begin->tv_usec) / 1000000;
		begin->tv_usec += 1000000 * nsec;
		begin->tv_sec -= nsec;
	}

	// Compute the time remaining to wait. 'tv_usec' is certainly positive.
	result->tv_sec = end->tv_sec - begin->tv_sec;
	result->tv_usec = end->tv_usec - begin->tv_usec;

	// Return 1 if result is negative
	return end->tv_sec < begin->tv_sec;
}

R_API void r_prof_start(struct r_prof_t *p) {
	tv *begin = &p->begin;
	p->result = 0.0;
	gettimeofday(begin, NULL);
}

R_API double r_prof_end(struct r_prof_t *p) {
	tv end, diff, *begin = &p->begin;
	int sign;
	gettimeofday (&end, NULL);
	sign = timeval_subtract (&diff, begin, &end);
	p->result = R_ABS (((double)(diff.tv_sec)
		+ ((double)diff.tv_usec / 1000000.)));
	return R_ABS (sign);
}

#endif
