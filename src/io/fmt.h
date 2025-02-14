#include <stdio.h>

#define FMT(buffer, fmt, ...) fmt_str((buffer), (fmt), __VA_ARGS__)

extern char *fmt_str(char * buffer, const char *restrict fmt, ...);