#define __AVR_ATtiny85__

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "utils.h"
#include "io/io.h"
#include "io/fmt.h"
#include "twi/uart.h"
#include "twi/i2c.h"

#define OLED_ADDR ((0x3C << 1) | 0) 
const uint8_t font5x7[96][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // (SPACJA)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x1C, 0x22, 0x41, 0x00, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x72, 0x49, 0x49, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x59, 0x09, 0x06}, // ?
    {0x3E, 0x41, 0x5D, 0x55, 0x1E}, // @
    {0x7E, 0x09, 0x09, 0x09, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x3A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x06, 0x08, 0x30, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    // itd... wszystkie znaki ASCII
};


//    {0x3E, 0x41, 0x49, 0x49, 0x3A}, // G
/*
0x3E    00111110
0x41    00100001
0x49    01001001
0x49    01001001
0x3A    00111010

(rotate)

||0x3E|0x41|0x49|0x49|0x3A|

↾|    |####|####|####|    |
⇂|    |####|####|####|    |
↾|####|    |    |    |####|
⇂|####|    |    |    |####|
↾|####|    |    |    |    |
⇂|####|    |    |    |    |
↾|####|    |####|####|####|
⇂|####|    |####|####|####|
↾|####|    |    |    |####|
⇂|####|    |    |    |####|
↾|####|####|    |    |####|
⇂|####|####|    |    |####|
↾|    |    |####|####|    |
⇂|    |    |####|####|    |
*/




void oled_init() {
    i2c_start();
    i2c_write_byte(OLED_ADDR); // (0x3C << 1) dla zapisu
    i2c_write_byte(0x00); // Tryb komend

    i2c_write_byte(0xAE); // Wyłącz OLED
    i2c_write_byte(0xA8); i2c_write_byte(0x3F); // Ustaw wysokość na 64 px
    i2c_write_byte(0xD3); i2c_write_byte(0x00); // Offset
    i2c_write_byte(0x40); // Start linii
    i2c_write_byte(0xA1); // Odwrócenie kolumn
    i2c_write_byte(0xC8); // Odwrócenie wierszy
    i2c_write_byte(0xDA); i2c_write_byte(0x12); // Konfiguracja pinów
    i2c_write_byte(0x81); i2c_write_byte(0x7F); // Kontrast
    i2c_write_byte(0xA4); // Włącz normalny tryb (nie całkowicie włączony)
    i2c_write_byte(0xA6); // Tryb normalny (nie inwersja)
    i2c_write_byte(0xD5); i2c_write_byte(0x80); // Częstotliwość zegara
    i2c_write_byte(0xD9); i2c_write_byte(0xF1); // Ustawienia ładowania
    i2c_write_byte(0xDB); i2c_write_byte(0x20); // Ustawienia VCOM
    i2c_write_byte(0x8D); i2c_write_byte(0x14); // Włącz ładowanie
    i2c_write_byte(0xAF); // Włącz OLED

    i2c_stop();
}

void oled_set_cursor(uint8_t x, uint8_t y) {
    i2c_start();
    i2c_write_byte(OLED_ADDR);
    i2c_write_byte(0x00); // Tryb komend

    i2c_write_byte(0xB0 + y);  // Wybór strony (wiersz)
    i2c_write_byte(0x00 + (x & 0x0F));  // Niska część adresu kolumny
    i2c_write_byte(0x10 + ((x >> 4) & 0x0F));  // Wysoka część adresu kolumny

    i2c_stop();
}
void oled_draw_char(uint8_t x, uint8_t y, char c) {
    oled_set_cursor(x, y); // Ustaw pozycję

    i2c_start();
    i2c_write_byte(OLED_ADDR); 
    i2c_write_byte(0x40); // Tryb danych

    for (uint8_t i = 0; i < 5; i++) {
        i2c_write_byte(font5x7[c - ' '][i]); // Pobranie znaku z tablicy fontów
    }
    i2c_write_byte(0x00); // Dodatkowy odstęp

    i2c_stop();
}
void oled_command(uint8_t cmd) {
    i2c_start();
    i2c_write_byte(OLED_ADDR);
    i2c_write_byte(0x00); // Tryb komendy
    i2c_write_byte(cmd); 
    i2c_stop();
}
void oled_write_text(uint8_t x, uint8_t y, const char *text) {
    while (*text) {
        oled_draw_char(x, y, *text);
        x += 6; // 5 pikseli + 1 piksel odstępu
        text++;
    }
}


void oled_fill_black() {
    i2c_start();
    i2c_write_byte(OLED_ADDR); // Adres OLED
    i2c_write_byte(0x00); // Tryb komend

    // Ustawienie kursora na początek ekranu
    i2c_write_byte(0x20); i2c_write_byte(0x00); // Kolumny 0-127
    i2c_write_byte(0x21); i2c_write_byte(0x00); i2c_write_byte(0x7F); // Kolumny 0-127
    i2c_write_byte(0x22); i2c_write_byte(0x00); i2c_write_byte(0xB7); // Wiersze 0-7 (cały ekran)

    i2c_stop();

    i2c_start();
    i2c_write_byte(OLED_ADDR); // Adres OLED
    i2c_write_byte(0x40); // Tryb danych

    // Wypełnianie ekranu (SSD1306 ma 8 stron po 128 bajtów)
    for (uint16_t i = 0; i < (128 * 8); i++) {
        i2c_write_byte(0x00); // Każdy bajt to 8 włączonych pikseli
    }

    i2c_stop();
}
int main(void) {
    init_uart();
    // _delay_ms(5000);
    i2c_init();
    oled_init();
    oled_fill_black();
    while (true) {
        //uart_receive(0); 
        // oled_show_dupa();
        oled_write_text(0, 0, "HELLO! G G G G");
        // oled_write_text(0, 2, "NO BUFFER");
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