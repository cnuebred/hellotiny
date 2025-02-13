## Pomoce naukowe :3

```
// kompilacja .c do .elf
avr-gcc -mmcu=attiny85 -Os -o main.elf main.c

// do wyjaśnienia co to wgle robi
avr-objcopy -O ihex -R .eeprom main.elf main.hex

//wgrywanie na arduino
avrdude -c arduino -p t85 -P COM7 -b 19200 -U flash:w:main.hex:i


$ sudo apt install make gcc-avr avr-libc avrdude

make -v
avr-gcc --version
avrdude -v

// to check https://codeandlife.com/2012/01/29/avr-attiny-usb-tutorial-part-3/
// https://www.gadgetronicx.com/attiny85-i2c-protocol-tutorial/
// https://tinusaur.org/2019/02/10/c-library-for-ssd1306-oled-display-and-attiny85/
```