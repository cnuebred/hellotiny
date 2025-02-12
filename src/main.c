#define __AVR_ATtiny85__


#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "utils.h"
#include "io/io.h"


int main(void) {
    // Ustaw PB0 jako wyjście (Tx)
    DDRB |= (1 << PB0);
    PORTB |= (1 << PB0); // Domyślny stan wysoki

    while (1) {
        //uart_receive(0);
        uart_transmit('H'); 
        uart_transmit('e');
        uart_transmit('l');
        uart_transmit('l');
        uart_transmit('o');
        uart_transmit('-');
        uart_transmit(PORTB);
        uart_transmit('\n');
        uart_transmit('\r');
        _delay_ms(1000);
    }
}