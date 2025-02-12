#ifndef	_TINY_UART
#define _TINY_UART

#include "io/io.h"

int get_parity(unsigned int n)
{
  int parity = 0;
  while (n)
  {
    parity = !parity;
    n = n & (n - 1);
  }
  return parity;
}




void uart_transmit(char data)
{
  // Zakładamy pin PB0 jako Tx
  PORTB &= ~(1 << PB0);         // Start bit (stan niski)
  _delay_us(UART_BIT_DELAY_US); // 1 bit = 1/9600 s (przy 9600 bps)

  for (uint8_t i = 0; i < 8; i++)
  {
    if ((data >> i) & 1)
    {
      set_pin(UART_PORT_TX UART_PIN_TX, HIGH)
    }
    else
    {
      set_pin(UART_PORT_TX UART_PIN_TX, LOW)
    }
    _delay_us(UART_BIT_DELAY_US);
  }

  // PORTB |= (get_parity((int)data) << PB0);

   set_pin(UART_PORT_TX UART_PIN_TX, HIGH)
  _delay_us(UART_BIT_DELAY_US);
}

void uart_receive(char data)
{
  // Zakładamy pin PB0 jako Tx
  while (1)
  {
    if (PORTB & (1 << PB1) == 1)
    {
      uart_transmit('R');
      uart_transmit('\n');
      uart_transmit('\r');
      break;
    }
  }
  // PORTB &= ~(1 << PB1); // Start bit (stan niski)
  // _delay_us(BIT_DELAY_US);       // 1 bit = 1/9600 s (przy 9600 bps)

  // for (uint8_t i = 0; i < 8; i++) {
  //     if ((data >> i) & 1) {
  //         PORTB |= (1 << PB1); // Bit wysoki
  //     } else {
  //         PORTB &= ~(1 << PB1); // Bit niski
  //     }
  //     _delay_us(BIT_DELAY_US);
  // }

  // //PORTB |= (get_parity((int)data) << PB0);

  // PORTB |= (1 << PB1); // Stop bit (stan wysoki)
  // _delay_us(BIT_DELAY_US);
}

#endif // _TINY_UART