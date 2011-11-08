#ifndef UTF8_H
#define UTF8_H

#include <wchar.h>

int utowc(wchar_t *wch, const char *s, int n);

int ustowcs(wchar_t *wch, const char *s, int n);

int wctou(char *s, const wchar_t wch, int n);

int wcstous(char *s, const wchar_t *wch, int n);

int uslen(const char *s);

#endif

