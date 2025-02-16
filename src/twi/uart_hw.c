#include <util/delay.h>
#include "uart.h"
#include "../utils.h"

void init_uart(){
  SET_BIT(DDRB, UART_PIN_TX);
  SET_BIT(UART_PORT_TX, UART_PIN_TX);
}
void terminate_uart(){
  CLR_BIT(DDRB, UART_PIN_TX);
  CLR_BIT(UART_PORT_TX, UART_PIN_TX);
}

void uart_transmit(char data)
{
  CLR_BIT(UART_PORT_TX, UART_PIN_TX);
  _delay_us(UART_BIT_DELAY_US); 

  for (uint8_t i = 0; i < 8; i++)
  {
    if ((data >> i) & 1)
    {
      SET_BIT(UART_PORT_TX, UART_PIN_TX);
    }
    else
    {
      CLR_BIT(UART_PORT_TX, UART_PIN_TX);
    }
    _delay_us(UART_BIT_DELAY_US);
  }

  SET_BIT(UART_PORT_TX, UART_PIN_TX);
  _delay_us(UART_BIT_DELAY_US);
}

