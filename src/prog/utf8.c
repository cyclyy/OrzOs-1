#include "utf8.h"
#include <string.h>

int utowc(wchar_t *wch, const char *s, int n)
{
        int ret;
	unsigned long conv;
        if (n < 1)
                return 0;
        if ((unsigned char)s[0] <= 0x7f) {
                ret = 1;
                conv = s[0]; 
        } else if ((unsigned char)s[0] >= 0xf0) {
                if (n < 4)
                        return 0;
                ret = 4;
                conv = ((s[0] & 0x7) << 18) + ((s[1] & 0x3f) << 12) + ((s[2] & 0x3f) << 6) + ((s[3] & 0x3f));
        } else if ((unsigned char)s[0] >= 0xe0) {
                if (n < 3)
                        return 0;
                ret = 3;
                conv = ((s[0] & 0xf) << 12) + ((s[1] & 0x3f) << 6) + (s[2] & 0x3f);
        } else if ((unsigned char)s[0] >= 0xc0) {
                if (n < 2)
                        return 0;
                ret = 2;
                conv = ((s[0] & 0x1f) << 6) + (s[1] & 0x3f);
        }
	*wch = conv;
        return ret;
}

int ustowcs(wchar_t *wch, const char *s, int n)
{
        int i = 0, j = 0;
        while (i < n - 1) {
                j = utowc(wch, s, 4);
                if (!j || !*wch)
                        break;
                wch++;
                i++;
                s += j;
        }
        *wch = 0;
        return i;
}


int wctou(char *s, const wchar_t wch, int n)
{
        unsigned long conv = wch;
        int ret;
        if (n < 1)
                return 0;
        if (conv <= 0x7f) {
                ret = 1;
                s[0] = conv;
        } else if (conv <= 0x7ff) {
                if (n < 2)
                        return 0;
                ret = 2;
                s[1] = (conv & 0x3f) | 0x80;
                conv >>= 6;
                s[0] = (conv & 0x1f) | 0xc0;
        } else if (conv <= 0xffff) {
                if (n < 3)
                        return 0;
                ret = 3;
                s[2] = (conv & 0x3f) | 0x80;
                conv >>= 6;
                s[1] = (conv & 0x3f) | 0x80;
                conv >>= 6;
                s[0] = (conv & 0xf) | 0xe0;
        } else if (conv <= 0x1fffff) {
                if (n < 4)
                        return 0;
                ret = 4;
                s[3] = (conv & 0x3f) | 0x80;
                conv >>= 6;
                s[2] = (conv & 0x3f) | 0x80;
                conv >>= 6;
                s[1] = (conv & 0x3f) | 0x80;
                conv >>= 6;
                s[0] = (conv & 0x7) | 0xf0;
        } else
                return 0;

        return ret;
                
}

int wcstous(char *s, const wchar_t *wch, int n)
{
        int i = 0, j = 0;
        while (i < n - 1) {
                j = wctou(s, *wch, n - i);
                if (!j || !*s)
                        break;
                wch++;
                i++;
                s += j;
        }
        *s = 0;
        return i;
}

int uslen(const char *s)
{
        int i = 0, j = 0, n = strlen(s);
        wchar_t wch;
        while (i < n) {
                j = utowc(&wch, s, 4);
                if (!j || !wch)
                        break;
                i++;
                s += j;
        }
        return i;
}

