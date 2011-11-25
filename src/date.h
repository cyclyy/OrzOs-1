#ifndef DATE_H
#define DATE_H

struct Date
{
    int year, month, day;
    int hour, minute, second;
};

long ticksFromDate(const struct Date *date);

void setDate(struct Date *date, long ticks);

#endif /* DATE_H */
