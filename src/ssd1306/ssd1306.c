#include "ssd1306.h"
#include "../twi/i2c.h"
#include <util/delay.h>
#include <avr/eeprom.h>

#define FONT5x7_WIDTH 5
#define FONT5x7_HIGHT 7

static const byte FONT5x7[96][5] PROGMEM = {
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
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x41, 0x3E}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x3A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x41, 0x41, 0x7F, 0x41, 0x41}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x06, 0x08, 0x30, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}  // Z
};

//    {0x3E, 0x41, 0x49, 0x49, 0x3A}, // G
/*
       [0 1 2 3 4 5 6 7]
0x3E    0 0 1 1 1 1 1 0
0x41    0 0 1 0 0 0 0 1
0x49    0 1 0 0 1 0 0 1
0x49    0 1 0 0 1 0 0 1
0x3A    0 0 1 1 1 0 1 0

(rotate)

|  |0x3E|0x41|0x49|0x49|0x3A|
____________________________
7  |    |####|####|####|    |
7__|____|####|####|####|____|
6  |####|    |    |    |####|
6__|####|____|____|____|####|
5  |####|    |    |    |    |
5__|####|____|____|____|____|
4  |####|    |####|####|####|
4__|####|____|####|####|####|
3  |####|    |    |    |####|
3__|####|____|____|____|####|
2  |####|####|    |    |####|
2__|####|####|____|____|####|
1  |    |    |####|####|    |
1__|____|____|####|####|____|
0  |    |    |    |    |    |
0__|____|____|____|____|____|
*/

const byte init_oled_set_list[] = {
    I2C_OLED_ADDRESS,
    OLED_COMMAND_MODE,
    OLED_SET_DISPLAY_OFF,
    OLED_SET_MULTIPLEX_RATIO_, 0x3F,
    OLED_SET_DISPLAY_OFFSET_, 0x00,
    OLED_DATA_MODE,
    OLED_SET_SEGMENT_RE_MAP_FOR,
    OLED_SET_COM_OUTPUT_SCAN_DIRECTION_FOR,
    OLED_SET_COM_PINS_HARDWARE_CONFIG_, 0x12,
    OLED_SET_CONTRAST_CONTROL_, 0x7F,
    OLED_ENTIRE_DISPLAY_ON__ENABLE_GGDRAM,
    OLED_SET_NORMAL_DISPLAY,
    OLED_SET_DISPLAY_CLOCK_DIVIDE_RATIO_OSCILLATOR_FREQUENCY_, 0xF0,
    OLED_SET_PRECHARGE_PERIOD_, 0xF1,
    OLED_SET_VCOM_DESELECT_LEVEL_, 0x20,
    OLED_CHARGE_PUMP_SETTINGS_, OLED_ENABLE_CHARGE_PUMP,
    OLED_SET_DISPLAY_ON};

void oled_init()
{
  send_package(init_oled_set_list, NELEMS(init_oled_set_list));
}

void oled_set_cursor(uint8_t x, uint8_t y)
{
  const byte set_cursor_set_list[] = {
      I2C_OLED_ADDRESS,
      OLED_COMMAND_MODE,
      OLED_SET_MEMORY_ADDRESING_MODE_, 0x10,
      OLED_SET_PAGE_START_ADDRESS(y),
      OLED_SET_LOWER_START_COL(x & 0x0F),
      OLED_SET_UPPER_START_COL((x >> 4) & 0x0F)};

  send_package(set_cursor_set_list, NELEMS(set_cursor_set_list));
}

void oled_set_pixels(uint8_t x, uint8_t y, byte column)
{
  oled_set_cursor(x, (uint8_t)(y / 8));

  const byte set_list[] = {
      I2C_OLED_ADDRESS,
      OLED_DATA_MODE,
      (uint8_t)(column),
  };
  send_package(set_list, NELEMS(set_list));
}

void oled_draw_char(uint8_t x, uint8_t y, char c, OLED_COLOR color)
{
  oled_set_cursor(x, y);

  i2c_start();
  i2c_write_byte(I2C_OLED_ADDRESS);
  i2c_write_byte(OLED_DATA_MODE);

  for (uint16_t i = 0; i < FONT5x7_WIDTH; i++)
  {
    byte bitmap_c = pgm_read_byte(&(FONT5x7[c - ' '][0]));
    i2c_write_byte(color ? bitmap_c : ~bitmap_c);
  }
  i2c_write_byte(0x00);

  i2c_stop();
}

void oled_write_text(uint8_t x, uint8_t y, const char *text, OLED_COLOR color)
{
  while (*text)
  {
    oled_draw_char(x, y, *text, color);
    x += FONT5x7_WIDTH + 1; // + 1 white separator
    text++;
  }
}

static void oled_set_plane(uint8_t x, uint8_t y, uint8_t size_x, uint8_t size_y)
{
  const byte fill_black_set_list[] = {
      I2C_OLED_ADDRESS,
      OLED_COMMAND_MODE,
      OLED_SET_MEMORY_ADDRESING_MODE_,
      OLED_SET_MEMORY_ADDRESING_MODE__PAGE,
      OLED_SET_COLUMN_ADDRESS__,
      x,
      (x + size_x),
      OLED_SET_PAGE_ADDRESS__,
      (0xB0 + y),
      (0xB0 + (y + size_y)),
  };

  send_package(fill_black_set_list, NELEMS(fill_black_set_list));
}

void oled_draw_bitmap(uint8_t x, uint8_t y, uint8_t size_x, uint8_t size_y,
                      const byte *bitmap, OLED_COLOR color)
{
  oled_set_plane(x, y, size_x - 1, size_y - 1);

  i2c_start();
  i2c_write_byte(I2C_OLED_ADDRESS);
  i2c_write_byte(OLED_DATA_MODE);

  for (uint16_t i = 0; i < size_y * size_x; i++)
  {
    byte bitmap_seg = pgm_read_byte(&bitmap[i]);
    i2c_write_byte(color ? bitmap_seg : ~bitmap_seg);
  }

  i2c_stop();
}

void oled_clear()
{

  oled_set_plane(
      0,
      0,
      OLED_DISPLAY_ADDRESSING_SIZE_WIDTH - 1,
      OLED_DISPLAY_ADDRESSING_SIZE_HIGHT - 1);

  i2c_start();
  i2c_write_byte(I2C_OLED_ADDRESS);
  i2c_write_byte(OLED_DATA_MODE);

  uint16_t full_screen_size = (OLED_DISPLAY_ADDRESSING_SIZE_HIGHT * OLED_DISPLAY_ADDRESSING_SIZE_WIDTH);
  for (uint16_t i = 0; i < full_screen_size; i++)
  {
    i2c_write_byte(0x00);
  }

  i2c_stop();
}
