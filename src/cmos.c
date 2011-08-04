#include "cmos.h"
#include "util.h"

#define CONVERT_FROM_DIGIT(x) ((statusB & CMOS_REG_SB_ENABLE_24HOURS)?x:(((x & 0xF0) >> 4)*10+(x&0x0F)))

static u8int rtcEnabled = 0;
static u8int statusB = 0;
static struct RTC rtc;

u8int cmosReadRegister(u8int regno)
{
    if (!rtcEnabled || !IS_VALID_CMOS_REG(regno)) {
        return 0;
    }
    outb(CMOS_BASE_PORT, regno);
    return inb(CMOS_DATA_PORT);
}

void initRTC()
{
    statusB = cmosReadRegister(CMOS_REG_SB);
    cmosGetRTC();
    rtcEnabled = 1;
}

const struct RTC *cmosGetRTC()
{
    if (!rtcEnabled) {
        return 0;
    }
    rtc.hour = CONVERT_FROM_DIGIT(cmosReadRegister(CMOS_REG_HOURS));
    rtc.minute = CONVERT_FROM_DIGIT(cmosReadRegister(CMOS_REG_MINUTES));
    rtc.second = CONVERT_FROM_DIGIT(cmosReadRegister(CMOS_REG_SECONDS));
    rtc.year = CONVERT_FROM_DIGIT(cmosReadRegister(CMOS_REG_CENTURY)) * 100 +
        CONVERT_FROM_DIGIT(cmosReadRegister(CMOS_REG_YEAR));
    rtc.month = CONVERT_FROM_DIGIT(cmosReadRegister(CMOS_REG_MONTH));
    rtc.day = CONVERT_FROM_DIGIT(cmosReadRegister(CMOS_REG_DATE_OF_MONTH));
    rtc.weekday = CONVERT_FROM_DIGIT(cmosReadRegister(CMOS_REG_DAY_OF_WEEK));
    return &rtc;
}

// vim: sw=4 sts=4 et tw=100
