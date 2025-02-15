#ifndef _TINY_UART
#define _TINY_UART

#include "../io/io.h"

extern void uart_transmit(char data);
extern void uart_send(char * data);
extern void init_uart();
extern void terminate_uart();

#endif // _TINY_UART