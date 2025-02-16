#include "uart.h"

void uart_send(char * data){
  while(*data){
    uart_transmit(*data);
    data++;
  }
}