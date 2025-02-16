#ifndef _TINY_I2C
#define _TINY_I2C

#include "../io_hw.h"
#include "../utils.h"

// =============================  I2C DEFINES  =============================

#define USISR_CLOCK_8_BITS                                        0b11110000
#define USISR_CLOCK_1_BIT                                         0b11111110



extern void send_package(byte * data);

extern void i2c_init();
extern void i2c_start();
extern void i2c_stop();
extern byte i2c_transfer(byte usisr_mask);
extern byte i2c_write_byte(byte data);
extern byte i2c_read_byte(byte nack);

#endif // _TINY_I2C