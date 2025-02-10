// #include <avr/io.h>
// #include <util/delay.h>

// char * tosend="test";
// #define I2C_SDA         PB0                   // serial data pin
// #define I2C_SCL         PB2                   // serial clock pin
// #define I2C_SDA_HIGH()  DDRB &= ~(1<<I2C_SDA) // release SDA   -> pulled HIGH by resistor
// #define I2C_SDA_LOW()   DDRB |=  (1<<I2C_SDA) // SDA as output -> pulled LOW  by MCU
// #define I2C_SCL_HIGH()  DDRB &= ~(1<<I2C_SCL) // release SCL   -> pulled HIGH by resistor
// #define I2C_SCL_LOW()   DDRB |=  (1<<I2C_SCL) // SCL as output -> pulled LOW  by MCU

// // I2C init function
// void I2C_init(void) {
//   DDRB  &= ~((1<<I2C_SDA)|(1<<I2C_SCL));  // pins as input (HIGH-Z) -> lines released
//   PORTB &= ~((1<<I2C_SDA)|(1<<I2C_SCL));  // should be LOW when as ouput
// }

// // I2C start transmission
// void I2C_start(uint8_t addr) {
//   I2C_SDA_LOW();                          // start condition: SDA goes LOW first
//   I2C_SCL_LOW();                          // start condition: SCL goes LOW second
//   I2C_write(addr);                        // send slave address
// }

// // I2C stop transmission
// void I2C_stop(void) {
//   I2C_SDA_LOW();                          // prepare SDA for LOW to HIGH transition
//   I2C_SCL_HIGH();                         // stop condition: SCL goes HIGH first
//   I2C_SDA_HIGH();                         // stop condition: SDA goes HIGH second
// }

// // I2C transmit one data byte to the slave, ignore ACK bit, no clock stretching allowed
// void I2C_write(uint8_t data) {
//   for(uint8_t i = 8; i; i--) {            // transmit 8 bits, MSB first
//     I2C_SDA_LOW();                        // SDA LOW for now (saves some flash this way)
//     if (data & 0x80) I2C_SDA_HIGH();      // SDA HIGH if bit is 1
//     I2C_SCL_HIGH();                       // clock HIGH -> slave reads the bit
//     data<<=1;                             // shift left data byte, acts also as a delay
//     I2C_SCL_LOW();                        // clock LOW again
//   }
//   I2C_SDA_HIGH();                         // release SDA for ACK bit of slave
//   I2C_SCL_HIGH();                         // 9th clock pulse is for the ACK bit
//   asm("nop");                             // ACK bit is ignored, just a delay
//   I2C_SCL_LOW();                          // clock LOW again
// }