#include "date.h"

#define EPOCH_YEAR  1970
#define NORM_YEAR_DAYS   365
#define LEAP_YEAR_DAYS   366

#define IS_LEAP_YEAR(x) ((((x)%4==0) && ((x)%100!=0)) || ((x)%400==0))

#define YEAR_DAYS(x) (IS_LEAP_YEAR(x) ? LEAP_YEAR_DAYS : NORM_YEAR_DAYS)

#define MONTH_DAYS(y, m) (MONTH_DAYS[m] + ((IS_LEAP_YEAR(y)&&(m==2)) ? 1 : 0))

static int MONTH_DAYS[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
};

long ticksFromDate(const Date *date)
{
    long i, days, seconds;
    days = 0;
    for (i = EPOCH_YEAR; i < date->year; i++)
        days += YEAR_DAYS(i);
    for (i = 1; i < date->month; i++)
        days += MONTH_DAYS(date->year, i);
    days += date->day;
    seconds = days*86400 + date->hour*3600 + date->minute*60 + date->second;
    return seconds*1000;
}

void setDate(Date *date, long ticks)
{
    long seconds, minutes, hours, days, i;
    seconds = ticks/1000;
    minutes = seconds/60;
    hours = minutes/60;
    days = hours/24;
    date->second = seconds%60;
    date->minute = minutes%60;
    date->hour = hours%24;
    date->year = EPOCH_YEAR;
    while (days >= YEAR_DAYS(date->year)) {
        days -= YEAR_DAYS(date->year);
        date->year++;
    }
    date->month = 1;
    while (days >= MONTH_DAYS(date->year, date->month)) {
        days -= MONTH_DAYS(date->year, date->month);
        date->month++;
    }
    date->day = days + 1;
}

