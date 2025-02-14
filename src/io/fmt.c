#include "fmt.h"

#define FMT(buffer, fmt, ...) fmt_str((buffer), (fmt), __VA_ARGS__)

char *fmt_str(char * buffer, const char *restrict fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);
    va_end(ap);

    return buffer;
}