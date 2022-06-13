#include "defs.h"
#include "config.h"
#include "util.h"

///////////////////////////////////////DATE TIME APIS//////////////////////////////////////
static const uint8 g_str_year_table[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const uint8 g_str_leap_year_table[] = { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


static uint8 iso8601_is_leap_year(uint16 year) {
	if((year % 400) == 0) return 1;
	if((year % 100) == 0) return 0;
	return ((year % 4) == 0);
}

LIB_API uint32 util_iso8601_to_filetime(struct datetime * dtime) {
	uint32 filetime = 0;
	uint8 * list_mm;
	uint16 year;
	uint8 i;
	uint16 days = 0;
	filetime = dtime->second + (dtime->minute * 60) + (dtime->hour * 3600);
	if(iso8601_is_leap_year(dtime->year)) list_mm = (uint8 *)g_str_leap_year_table;
	else list_mm = (uint8 *)g_str_year_table;
	//convert to days
	for(i=0;i<dtime->month;i++) {
		days += list_mm[i];
	}
	days += (dtime->date - 1);
	dtime->days = days;
	filetime += (dtime->days * 86400);
	if(dtime->year >= 1970 && dtime->year <2049) {
		year = dtime->year - 1970;
		//if(dtime->days >= 365) year--;
		//filetime += (year * 31557600);
		for (int i = 0;i < year;i++) {
			if (iso8601_is_leap_year(1970 + i))
				filetime += 31622400;
			else
				filetime += 31536000;
		}
	}
	return filetime;
};

LIB_API uint8 util_iso8601_from_filetime(struct datetime * dtime, long filetime) {
	long mres = 0;
	uint8 * list_mm;
	uint8 i;
	long residue;
	uint16 days;
	uint16 y;
	uint16 year = filetime / 31557600;
	for (y = 0; y < 100;y++) {
		if (iso8601_is_leap_year(1970 + y)) { 
			list_mm = (uint8 *)g_str_leap_year_table; 
			if(filetime > 31622400) 
				filetime -= 31622400;
			else 
				break;
		} else { 
			list_mm = (uint8 *)g_str_year_table;
			if (filetime > 31622400)
				filetime -= 31536000;
			else
				break;
		}
	}
	dtime->year = 1970 + y;
restart_calc:
	//if(iso8601_is_leap_year(dtime->year)) { max_days = 366;}
	//else { max_days = 365; }
	residue = filetime;
	dtime->days = (residue / 86400);
	days = dtime->days;
	//convert to month-date
	for(i=0;i<12;++i) {
		//mres = ;
		if(days <= (list_mm[i+1])) {
			break;
		} else 
			days -= list_mm[i+1];
		 
	}
	if(days == list_mm[i+1]) { days = 0; i += 1;}		//in-case days = max_days_in_month
	dtime->month = (i+1);
	dtime->date = (days + 1);
	//dtime->date = (residue / 86400);
	//dtime->date ++;
	residue = residue % 86400;
parse_time:
	dtime->hour = residue / 3600;
	residue = residue % 3600;
	dtime->minute = residue / 60;
	dtime->second = residue % 60;
	dtime->tz = 0;
	//if(iso8601_is_leap_year(dtime->year)) { list_mm = (uint8 *)g_str_leap_year_table; }
	//else { list_mm = (uint8 *)g_str_year_table; }
	return 0;
};

LIB_API uint32 util_iso8601_decode(char * str) {
	char c;
	uint8 state = 0;
	uint8 index = 0;
	uint16 temp;
	uint8 i;
	uint8 * list_mm;
	uint16 days = 0;
	uint8 negate_tz = 0;
	struct datetime dtime;
	uint16 field[7]={0,0,0,0,0,0,0};
	uint16 flen[] = {4,2,2,2,2,2,4};
	while((c = *str++) != 0) {
		switch(c) {
			case '-':
				if(state == 6) {
					negate_tz = 1;
				}
			case ':': break;

			case '+': break;
			case ' ': 
			case 'T': 
				state = 3;		//time
				break;
			case 'Z': break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				temp = field[state];
				temp *= 10;
				temp += (c & 0x0F);
				field[state] = temp;
				index ++;
				if(index == flen[state]) {
					state ++;
					index = 0;
				}
				break;
		}
	}
	if(negate_tz) {
		field[6] = 0 - field[6];
	}
	dtime.year = field[0];
	dtime.month = field[1];
	dtime.date = field[2];
	dtime.hour = field[3];
	dtime.minute = field[4];
	dtime.second = field[5];
	dtime.tz = field[6];
	if(iso8601_is_leap_year(dtime.year)) list_mm = (uint8 *)g_str_leap_year_table;
	else list_mm = (uint8 *)g_str_year_table;
	//convert from month-date
	for(i=0;i<dtime.month;i++) {
		days += list_mm[i];
	}
	days += (dtime.date -1);
	dtime.days = days;
	//state = 0;
	return util_iso8601_to_filetime(&dtime);
}

LIB_API void util_iso8601_encode(struct datetime * dtime, uint8 * buffer) {
	uint8 c = '+';
	if(dtime->tz < 0) c = '-';
	sprintf((char *)buffer, "%04d-%02d-%02d %02d:%02d:%02d%c%02d", dtime->year, dtime->month, dtime->date, dtime->hour, dtime->minute, dtime->second, c, dtime->tz);
}
////////////////////////////////////END OF DATE TIME APIS//////////////////////////////////////
