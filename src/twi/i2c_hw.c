
#include <util/delay.h>
#include "i2c.h"
#include "../utils.h"

void i2c_init()
{
	SET_BIT(DDRB, I2C_PIN_SDA);
	SET_BIT(DDRB, I2C_PIN_SCL);

	SET_BIT(I2C_PORT, I2C_PIN_SCL);

	I2C_PORT |= (1 << I2C_PIN_SDA);

	USIDR = 0xFF;

	USICR = (1 << USIWM1) | (1 << USICS1) | (1 << USICLK);

	USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) |
					(1 << USIDC) |		
					(0x0 << USICNT0);
}

void i2c_start()
{
	SET_BIT(I2C_PORT, I2C_PIN_SDA);
	SET_BIT(I2C_PORT, I2C_PIN_SCL);
	
	while (!(I2C_PORT & (1 << I2C_PIN_SCL)));
	
	CLR_BIT(I2C_PORT, I2C_PIN_SDA);
	
	_delay_us(I2C_WAIT_LONG);
	
	CLR_BIT(I2C_PORT, I2C_PIN_SCL);
	SET_BIT(I2C_PORT, I2C_PIN_SDA);
}

void i2c_stop()
{
	CLR_BIT(I2C_PORT, I2C_PIN_SDA);

	SET_BIT(I2C_PORT, I2C_PIN_SCL); 

	while (!(PINB & (1 << I2C_PIN_SCL)));

	_delay_us(I2C_WAIT_LONG);

	SET_BIT(I2C_PORT, I2C_PIN_SDA); 

	_delay_us(I2C_WAIT_SHORT);
}

byte i2c_transfer(byte usisr_mask)
{
	CLR_BIT(I2C_PORT, I2C_PIN_SCL);

	USISR = usisr_mask;
	do
	{
		_delay_us(I2C_WAIT_LONG);
		SET_BIT(USICR, USITC);
		while (!(PINB & (1 << I2C_PIN_SCL))); 
		_delay_us(I2C_WAIT_SHORT);
		SET_BIT(USICR, USITC);
	} while (!(USISR & (1 << USIOIF)));
	_delay_us(I2C_WAIT_LONG);

	byte temp = USIDR;
	USIDR = 0xFF;

	return temp;
}

byte i2c_write_byte(byte data)
{
	USIDR = data;
	i2c_transfer(USISR_CLOCK_8_BITS);

	CLR_BIT(DDRB, I2C_PIN_SDA);
	byte nack = i2c_transfer(USISR_CLOCK_1_BIT);
	SET_BIT(DDRB, I2C_PIN_SDA);

	return nack;
}


// nack (1 = will read another one, 0 = stop sending)
byte i2c_read_byte(byte nack)
{
	CLR_BIT(DDRB, I2C_PIN_SDA);
	byte data = i2c_transfer(USISR_CLOCK_8_BITS);
	SET_BIT(DDRB, I2C_PIN_SDA);

	USIDR = nack;
	i2c_transfer(USISR_CLOCK_1_BIT);

	return data;
}