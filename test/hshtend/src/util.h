#ifndef UTIL_H
#define UTIL_H

unsigned long sdbm(const char *str);
long long getnc(char *dst, long long n);
long long gettokn(char *src);
long long strtokn(char *src, const char *const delim, char **token);

#endif