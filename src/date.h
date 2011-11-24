#ifndef DATE_H
#define DATE_H

typedef struct
{
    int year, month, day;
    int hour, minute, second;
} Date;

long ticksFromDate(const Date *date);

void setDate(Date *date, long ticks);

#endif /* DATE_H */
