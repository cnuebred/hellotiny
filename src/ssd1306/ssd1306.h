#ifndef _TINY_SSD1306
#define _TINY_SSD1306

#include <avr/pgmspace.h>
#include "../utils.h"

/*
  0* stands for WRITE in i2c
  0x3C - 0011 1100 as address in i2c
  (0x3C << 1)              0111 1000 
  ((0x3C << 1) | 0)        0111 1000 

  I2C {start condition}[address 7-bit][R/W 1-bit][ack][data sequence]{stop condition}
  R/W => Read-1 | Write-0
*/
#define I2C_OLED_ADDRESS ((OLED_ADDRESS << 1) | 0) 

#define OLED_DISPLAY_SIZE_WIDTH 128
#define OLED_DISPLAY_SIZE_HIGHT 64

#define OLED_DISPLAY_ADDRESSING_SIZE_HIGHT (OLED_DISPLAY_SIZE_HIGHT/8)
#define OLED_DISPLAY_ADDRESSING_SIZE_WIDTH OLED_DISPLAY_SIZE_WIDTH

// ==========================  OLED I2C COMMANDS  ==========================

#define OLED_COMMAND_MODE                                               0x00
#define OLED_DATA_MODE                                                  0x40
#define OLED_SET_MEMORY_ADDRESING_MODE_                                 0x20
#define OLED_SET_MEMORY_ADDRESING_MODE__HORIZONTAL                      0x00
#define OLED_SET_MEMORY_ADDRESING_MODE__VERTICAL                        0x01
#define OLED_SET_MEMORY_ADDRESING_MODE__PAGE                            0x10
#define OLED_SET_COLUMN_ADDRESS__                                       0x21
#define OLED_SET_PAGE_ADDRESS__                                         0x22

// range: 0x40 ~ 0x7F [0 ~ 63]
#define OLED_SET_DISPLAY_START_LINE(value)          (0x40 + ((value) % 0x7F))
// range: 0xB0 ~ 0xB7 [0 ~ 7]
#define OLED_SET_PAGE_START_ADDRESS(value)          (0xB0 + ((value) % 0xB7)) 
// range: 0x00 ~ 0x0F [0 ~ 15]
#define OLED_SET_LOWER_START_COL(value)             (0x00 + ((value) % 0x10)) 
// range: 0x10 ~ 0x1F [0 ~ 15]
#define OLED_SET_UPPER_START_COL(value)             (0x10 + ((value) % 0x20)) 

// range: 0x00 ~ 0xFF [0 ~ 255]
#define OLED_SET_CONTRAST_CONTROL_                                      0x81
#define OLED_SET_SEGMENT_RE_MAP_FOR                                     0xA1
#define OLED_SET_SEGMENT_RE_MAP_REV                                     0xA0
#define OLED_ENTIRE_DISPLAY_ON__ENABLE_GGDRAM                           0xA4
#define OLED_ENTIRE_DISPLAY_ON__FORCE                                   0xA5
#define OLED_SET_NORMAL_DISPLAY                                         0xA6
#define OLED_SET_INVERSE_DISPLAY                                        0xA7
#define OLED_CHARGE_PUMP_SETTINGS_                                      0x8D
#define OLED_ENABLE_CHARGE_PUMP                                         0x14
#define OLED_DISABLE_CHARGE_PUMP                                        0x10

//  range: 0x10 ~ 0x3F [0 ~ 47]
#define OLED_SET_MULTIPLEX_RATIO_                                       0xA8
#define OLED_SET_DISPLAY_ON                                             0xAF
#define OLED_SET_DISPLAY_OFF                                            0xAE

#define OLED_SET_COM_OUTPUT_SCAN_DIRECTION_FOR                          0xC8
#define OLED_SET_COM_OUTPUT_SCAN_DIRECTION_REV                          0xC0

// range: 0x00 ~ 0x3F [0 ~ 63]
#define OLED_SET_DISPLAY_OFFSET_                                        0xD3
#define OLED_SET_DISPLAY_CLOCK_DIVIDE_RATIO_OSCILLATOR_FREQUENCY_       0xD5

/*
  A[3:0] : Phase 1 period of up to 15 DCLK clocks 0 is invalid entry
  (RESET=2h)
  A[7:4] : Phase 2 period of up to 15 DCLK clocks 0 is invalid entry
  (RESET=2h) 
*/
#define OLED_SET_PRECHARGE_PERIOD_                                      0xD9

/*
  COM pin configuration A[4]:
  - Sequential: 0
  - Alternative: 1

  COM Left/Right remap A[5]:
  - Disable: 0
  - Enable: 1

  byte template: `00[5][4] 0010`
  example: Sequential & Disable
  `0000 0010` == 0x02

  default value: 0x12
*/
#define OLED_SET_COM_PINS_HARDWARE_CONFIG_                              0xDA

/*
  Set VCOMH Deselect Level
  0 A6 A5 A4 0 0 0 0 

  A[6:4]:
  - 000b ~ 0.65xVcc
  - 010b ~ 0.77xVcc (RESET)
  - 011b ~ 0.83xVcc

*/
#define OLED_SET_VCOM_DESELECT_LEVEL_                                   0xDB
#define OLED_NOP                                                        0xE3

typedef enum {
  OLED_BLACK = 0,
  OLED_WHITE = 1,
} OLED_COLOR;



extern void oled_init();

/*
  x - number of column
  y - number of row

 Why so there is lower and upper start col pointer?
 That was confused to me... It's basically this come from 
 history of OLED controllers.
 There were only 4-bits registers for addresses, so programmers needs 
 to address columns by 7-bit address splitted into two commands:
 - lower
 - upper

 simple as budowa cepa:
 - if we want to set address for 34's column, which is 
 35 -> 00100011 -> 0x23
 so... we have two parts 0010 and 0011
 - lower command starts with 0x00 ~ 0x0F
 - upper commands starts with 0x10 ~ 0x1F

 for lower part we need 0011 -> lower command looks like 0x03 + [0x00]
 for upper part we need 0010 -> upper command looks like 0x02 + [0x10]
 
 there we go :3
*/
extern void oled_set_cursor(uint8_t x, uint8_t y);

extern void oled_draw_char(uint8_t x, uint8_t y, char c, OLED_COLOR color);

extern void oled_set_pixels(uint8_t x, uint8_t y, byte column);

extern void oled_write_text(uint8_t x, uint8_t y, const char *text, OLED_COLOR color);

extern void oled_draw_bitmap(
  uint8_t x, uint8_t y, uint8_t size_x, uint8_t size_y, 
  const byte *bitmap, OLED_COLOR color
);

extern void oled_clear();

#endif // _TINY_SSD1306