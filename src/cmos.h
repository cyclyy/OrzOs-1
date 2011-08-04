#ifndef CMOS_H
#define CMOS_H

#include "sysdef.h"

#define CMOS_BASE_PORT 0x70
#define CMOS_DATA_PORT 0x71

#define CMOS_REG_SECONDS         0x0
#define CMOS_REG_SECONDS_ALARM   0x1
#define CMOS_REG_MINUTES         0x2
#define CMOS_REG_MINUTES_ALARM   0x3
#define CMOS_REG_HOURS           0x4
#define CMOS_REG_HOURS_ALARM     0x5
#define CMOS_REG_DAY_OF_WEEK     0x6
#define CMOS_REG_DATE_OF_MONTH   0x7
#define CMOS_REG_MONTH           0x8
#define CMOS_REG_YEAR            0x9
#define CMOS_REG_SA              0xa
#define CMOS_REG_SB              0xb
#define CMOS_REG_SC              0xc
#define CMOS_REG_SD              0xd
#define CMOS_REG_CENTURY         0x32

#define CMOS_REG_SB_ENABLE_24HOURS       2
#define CMOS_REG_SB_ENABLE_BINARY_MODE   4

#define IS_VALID_CMOS_REG(x) ((x>=0) && (x<=0x33))

enum Weekday {
    Sunday = 1,Monday, Tuesday, Wendesday, Thursday, Friday, Saturday,
};

struct RTC
{
    u8int hour, minute, second;
    u16int year;
    u8int month, day;
    enum Weekday weekday;
};

u8int cmosReadRegister(u8int regno);

const struct RTC *cmosGetRTC();

void initRTC();

#endif

