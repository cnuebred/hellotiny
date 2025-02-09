## Pomoce naukowe :3

```
// kompilacja .c do .elf
avr-gcc -mmcu=attiny85 -Os -o main.elf main.c

// do wyjaśnienia co to wgle robi
avr-objcopy -O ihex -R .eeprom main.elf main.hex

//wgrywanie na arduino
avrdude -c arduino -p t85 -P COM7 -b 19200 -U flash:w:main.hex:i
```