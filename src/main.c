#define __AVR_ATtiny85__

#include "io_hw.h"
#include "twi/i2c.h"
#include "twi/uart.h"
#include "utils.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "ssd1306/ssd1306.h"

volatile int awake_INT = 0;
volatile int left_button_press = 0;
volatile int right_button_press = 0;



/*
  ALGO
  + works on battery

  - program in deep sleep
  |
  - interrupt by one button
    - run menu
    |
    - set previous settings or provide new one
    |
    - if previous settings \
    -> run program pomodoro (countdown first time, then [process], \
       countdown second time then [proecess], repeat)
    |
    if provide new one -> run config menu (set first time, set second time, set
  repeat's number)
    |
    while program pomodoro (carusel* menu) -> stop/start, reset, config
*/

/*
  SCHEME

    BUTTON LEFT + BUTTON RIGHT
        |
    ATTINY85 - BATTERY
        |
     SSD1306

*/

void init_PCINT() {
  CLR_BIT(DDRB, PIN4);
  SET_BIT(PORTB, PIN4);

  SET_BIT(GIMSK, PCIE);

  SET_BIT(PCMSK, PIN4);
}

ISR(PCINT0_vect) {
  if (!(PINB & (1 << PB4))) {
    awake_INT = true;
  }
}

void sleep() {
  SET_BIT(MCUCR, SE | SM1);
  CLR_BIT(MCUCR, SM0);
}

void display_menu(){

}

void init_buttons(){

}

void buttons_management(){
if (!(PINB & (1 << PBX0))) {
    awake_INT = true;
  }
if (!(PINB & (1 << PBX1))) {
    awake_INT = true;
  }
}


int main(void) {
  init_PCINT();
  i2c_init();

  oled_init();
  oled_clear();

  while (true) {
    if(awake_INT){

    }else{

    }
  }
}