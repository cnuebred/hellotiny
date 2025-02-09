#define __AVR_ATtiny85__

#include <avr/io.h>
#include <util/delay.h>


#define true 1
#define false 0

int main(void){
	DDRB |= (1 << PB0);
	while(true){
		PORTB ^= (1 << PB0);
		_delay_ms(500);
	}
}
