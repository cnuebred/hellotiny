
#include "i2c.h"


void send_package(byte * data){
  i2c_start();
  while(*data){
    i2c_write_byte(*data);
    data++;
  }
  i2c_stop();
}