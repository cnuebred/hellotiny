#ifndef _TINY_I2C
#define _TINY_I2C

#include "../io/io.h"

#define PIN_SDA PB0
#define PIN_SCL PB2

#define WAIT_LONG 5  // 4,7us
#define WAIT_SHORT 4 // 4,0us

// USISR mask
#define USISR_CLOCK_8_BITS 0b11110000
#define USISR_CLOCK_1_BIT 0b11111110

extern void i2c_init();
extern void i2c_start();
extern void i2c_stop();
extern unsigned char i2c_transfer(unsigned char usisr_mask);
extern unsigned char i2c_write_byte(unsigned char data);
extern unsigned char i2c_read_byte(unsigned char nack);

#endif // _TINY_I2C