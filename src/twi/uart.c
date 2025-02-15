#include <util/delay.h>
#include "assert.h"
#include "uart.h"
#include "../utils.h"

// static int get_parity(unsigned int n)
// {
//   int parity = 0;
//   while (n)
//   {
//     parity = !parity;
//     n = n & (n - 1);
//   }
//   return parity;
// }

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

void uart_send(char * data){
  assert(data);
  uint8_t index = 0;

  while(true){
    if(data[index] == '\0'){
      break;
    }
    uart_transmit(data[index]);
    index += 1;
  }

}

// void uart_receive(char data)
// {
//   // Zakładamy pin PB0 jako Tx
//   while (1)
//   {
//     if (PORTB & (1 << PB1) == 1)
//     {
//       uart_transmit('R');
//       uart_transmit('\n');
//       uart_transmit('\r');
//       break;
//     }
//   }
//   // PORTB &= ~(1 << PB1); // Start bit (stan niski)
//   // _delay_us(BIT_DELAY_US);       // 1 bit = 1/9600 s (przy 9600 bps)

//   // for (uint8_t i = 0; i < 8; i++) {
//   //     if ((data >> i) & 1) {
//   //         PORTB |= (1 << PB1); // Bit wysoki
//   //     } else {
//   //         PORTB &= ~(1 << PB1); // Bit niski
//   //     }
//   //     _delay_us(BIT_DELAY_US);
//   // }

//   // //PORTB |= (get_parity((int)data) << PB0);

//   // PORTB |= (1 << PB1); // Stop bit (stan wysoki)
//   // _delay_us(BIT_DELAY_US);
// }

