#include "i2c.h"

/*
  Why `_size_t size`? 
   - https://stackoverflow.com/a/10349610
*/

void send_package(const byte * data, _size_t size){
  i2c_start();
  for (int i = 0; i < size; i++){
    i2c_write_byte(data[i]);
  }
  i2c_stop();
}