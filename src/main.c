#define __AVR_ATtiny85__

#include <avr/io.h>
#include <util/delay.h>
#include "utils.h"
#include "io_hw.h"
#include "twi/uart.h"
#include "twi/i2c.h"

#include "ssd1306/ssd1306.h"


int main(void)
{
    i2c_init();

    oled_init();
    oled_fill_black();
    while (true)
    {
        oled_fill_black();
        oled_write_text(34, 3, "HELLO TINY");
        _delay_ms(300);

        oled_fill_black();
        oled_write_text(34, 5, "HELLO TINY");
        _delay_ms(300);

        oled_fill_black();
        oled_write_text(15, 2, "HELLO TINY");
        _delay_ms(300);

        oled_fill_black();
        oled_write_text(40, 6, "HELLO TINY");
        _delay_ms(300);
    }
}