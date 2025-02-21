#ifndef _TINY_UTILS
#define _TINY_UTILS

// ==============================  CODE UTILS  ==============================

#define true                                                             0x01
#define false                                                            0x00

// ===============================  TYPEDEFS  ===============================

typedef unsigned char byte;
typedef unsigned long _size_t;

#define NELEMS(x)                                (sizeof(x) / sizeof((x)[0]))

#endif // _TINY_UTILS
