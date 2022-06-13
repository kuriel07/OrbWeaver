#include "defs.h"
#include "config.h"

#ifndef UTIL__H
struct ticktime {
	long sec;
	long ns;
};

struct datetime {
	uint8 second;
	uint8 minute;
	uint8 hour;
	uint8 day;
	uint8 date;
	uint8 month;
	uint16 year;	
	uint16 days;			//days of year
	int8 tz;				//timezone
};
typedef struct ticktime ticktime;
typedef struct ticktime * ticktime_p;
typedef struct datetime datetime;
typedef struct datetime * datetime_p;

LIB_API uint32 util_iso8601_to_filetime(struct datetime * dtime);
LIB_API uint8 util_iso8601_from_filetime(struct datetime * dtime, long filetime) ;
LIB_API uint32 util_iso8601_decode(char * str);
LIB_API void util_iso8601_encode(struct datetime * dtime, uint8 * buffer);
#define UTIL__H
#endif
