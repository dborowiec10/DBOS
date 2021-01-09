#ifndef SYS_TIME_H
#define SYS_TIME_H
#include <sys/types.h>

struct timeval {
	time_t		tv_sec;			/* seconds */
	long		tv_usec;		/* microseconds */
};

#endif
