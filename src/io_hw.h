#ifndef _TINY_IO
#define _TINY_IO

#include <avr/io.h>

// =============================  CLOCK STUFFS  =============================

#define F_CPU                                                       8000000UL
#define UART_BAUDRATE                                                    9600
#define UART_BIT_DELAY_US                     ((1000000 / UART_BAUDRATE) * 2)

#define I2C_WAIT_LONG                                                       5
#define I2C_WAIT_SHORT                                                      4

// =============================  PORTS STUFFS  =============================

#define UART_PORT_TX                                                    PORTB
#define UART_PIN_TX                                                       PB1

#define I2C_PORT                                                        PORTB
#define I2C_PIN_SDA                                                       PB0
#define I2C_PIN_SCL                                                       PB2

// ==============================  IO OPS HW  ===============================

#define SET_BIT(PORT, PIN)                           ((PORT) |= (1 << (PIN)))
#define CLR_BIT(PORT, PIN)                          ((PORT) &= ~(1 << (PIN)))

#endif // _TINY_IO