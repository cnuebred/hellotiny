#ifndef _TINY_IO
#define _TINY_IO

#include <avr/io.h>

#define F_CPU 8000000UL

#define UART_BAUDRATE 9600
#define UART_BIT_DELAY_US ((1000000 / UART_BAUDRATE) * 2)

#define UART_PORT_TX PORTB
#define UART_PIN_TX PB1

// ===============================================================

#define LOW 0
#define HIGH 1

#define SET_BIT(PORT, PIN)  ((PORT) |= (1 << (PIN)))
#define CLR_BIT(PORT, PIN)  ((PORT) &= ~(1 << (PIN)))



#endif // _TINY_IO