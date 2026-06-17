#define __AVR_ATtiny85__

#include "io_hw.h"
#include "twi/i2c.h"
#include "twi/uart.h"
#include "utils.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "ssd1306/ssd1306.h"

volatile int awake_INT = 0;
volatile int is_awake = 0;
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

void print_uart_binary(uint8_t data)
{
  for (int i = 7; i >= 0; i--)
  {
    if ((data >> i) & 1)
    {
      uart_send("1");
    }
    else
    {
      uart_send("0");
    }
  }
  uart_send("\n\r");
}

void init_PCINT()
{
  uart_send("init PCINT\n\r");
  CLR_BIT(DDRB, PIN4);
  print_uart_binary(DDRB);
  SET_BIT(PORTB, PIN4);
  print_uart_binary(PORTB);

  SET_BIT(GIMSK, PCIE);
  print_uart_binary(GIMSK);

  SET_BIT(PCMSK, PIN4);
  print_uart_binary(PCMSK);

  sei();

  uart_send("init PCINT\n\r");
}

ISR(PCINT0_vect)
{
  if (!is_awake && !(PINB & (1 << PB4)))
  {
    left_button_press = true;
    right_button_press = false;

    awake_INT = true;
    is_awake = true;
  }
}

void sleep()
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
  sleep_disable();
}

void display_menu()
{
}

void init_buttons()
{
  uart_send("init buttons\n\r");
  CLR_BIT(DDRB, PIN3);
  print_uart_binary(DDRB);
  SET_BIT(PORTB, PIN3);
  print_uart_binary(PORTB);
}

void buttons_management()
{
  if (!right_button_press && !(PINB & (1 << PIN3)))
  {
    is_awake = false;
    right_button_press = true;
    left_button_press = false;
    uart_send(" PIN3!\n\r");
    oled_clear_all();
    oled_write_text(10, 3, "SLEEP", OLED_WHITE);
    sleep();
  }
  if (!left_button_press && !(PINB & (1 << PIN4)))
  {
    left_button_press = true;
    right_button_press = false;
    uart_send(" PIN4!\n\r");
    oled_clear_all();

    is_awake = true;
  }
}

int main(void)
{
  init_uart();
  init_buttons();
  for (int i = 0; i < 10; i++)
  {
    print_uart_binary(i);
    uart_send(" Hello, Tiny!\n\r");
    _delay_ms(150);
  }

  print_uart_binary(GIMSK);
  print_uart_binary(PCMSK);

  init_PCINT();
  i2c_init();

  oled_init();
  oled_clear_all();

  uart_send("START\n\r");

  while (true)
  {
    if (is_awake)
    {
      oled_clear_all();

      if (awake_INT)
      {
        oled_clear_all();
      }
      awake_INT = false;
      oled_write_text(10, 5, "PRESSED", OLED_WHITE);
      buttons_management();
    }
    else
    {
      oled_clear_all();
      oled_write_text(10, 3, "NONE", OLED_WHITE);
    }
    _delay_ms(100);
  }
}