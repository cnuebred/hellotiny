#define __AVR_ATtiny85__

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "utils.h"
#include "io/io.h"
#include "io/fmt.h"
#include "twi/i2c.h"
#include "twi/uart.h"

void oled_init() {
    i2c_start();
    i2c_write_byte(0x3C); // Adres OLED (zależy od wersji, może być 0x3C lub 0x3D)
    i2c_write_byte(0x00); // Tryb komend

    i2c_write_byte(0xAE); // Wyłącz wyświetlacz
    i2c_write_byte(0xD5); i2c_write_byte(0x80); // Set Display Clock Divisor
    i2c_write_byte(0xA8); i2c_write_byte(0x3F); // Set Multiplex Ratio (dla 128x64: 0x3F, dla 128x32: 0x1F)
    i2c_write_byte(0xD3); i2c_write_byte(0x00); // Set Display Offset
    i2c_write_byte(0x40); // Start Line
    i2c_write_byte(0x8D); i2c_write_byte(0x14); // Enable charge pump regulator
    i2c_write_byte(0x20); i2c_write_byte(0x00); // Memory mode: horizontal addressing
    i2c_write_byte(0xA1); // Set Segment Re-map (normal)
    i2c_write_byte(0xC8); // Set COM Output Scan Direction (normal)
    i2c_write_byte(0xDA); i2c_write_byte(0x12); // Set COM Pins Hardware Configuration
    i2c_write_byte(0x81); i2c_write_byte(0xCF); // Set Contrast
    i2c_write_byte(0xD9); i2c_write_byte(0xF1); // Set Pre-charge Period
    i2c_write_byte(0xDB); i2c_write_byte(0x40); // Set VCOMH Deselect Level
    i2c_write_byte(0xA4); // Enable display RAM
    i2c_write_byte(0xA6); // Normal display (A7 = inverse)
    i2c_write_byte(0xAF); // Włącz wyświetlacz

    i2c_stop();
}
const uint8_t font5x7[][5] = {
    {0x7C, 0x44, 0x44, 0x44, 0x38}, // D
    {0x7C, 0x54, 0x54, 0x54, 0x28}, // U
    {0x7C, 0x04, 0x7C, 0x04, 0x7C}, // P
    {0x7C, 0x14, 0x14, 0x14, 0x08}  // A
};
void oled_draw_char(uint8_t x, uint8_t y, char c) {
    i2c_start();
    i2c_write_byte(0x78); // Adres OLED
    i2c_write_byte(0x40); // Tryb danych

    for (uint8_t i = 0; i < 5; i++) {
        i2c_write_byte(font5x7[c - 'D'][i]); // Pobierz znak z tablicy
    }

    i2c_stop();
}

void oled_write_text(uint8_t x, uint8_t y, const char *text) {
    while (*text) {
        oled_draw_char(x, y, *text);
        x += 6; // Przeskok o szerokość znaku + odstęp
        text++;
    }
}

void oled_show_dupa() {
    oled_write_text(0, 0, "DUPA");
}

int main(void) {
    // SET_BIT(DDRB, UART_PIN_TX);
    // SET_BIT(UART_PORT_TX, UART_PIN_TX);
    _delay_ms(5000);
    i2c_init();
    oled_init();
    oled_show_dupa();

    while (true) {
        //uart_receive(0); 
        uart_send(FMT((char[128]){0}, "Hello - %02x\r\n", &PORTB));
        // uart_transmit('H'); 
        // uart_transmit('e');
        // uart_transmit('l');
        // uart_transmit('l');
        // uart_transmit('o');
        // uart_transmit('-');
        // uart_transmit(&PORTB);
        // uart_transmit('\n');
        // uart_transmit('\r');
        _delay_ms(1000);
    }
}