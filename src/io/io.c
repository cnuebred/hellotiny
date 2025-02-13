#include "io.h"
#include "assert.h"


#define assert_check_bit(value) assert(value == HIGH || value == LOW)


void set_pin(port_t port, pin_t pin, bit_t value){
  assert_check_bit(value);
  
  if (value == LOW){
    port &= ~(1 << pin); 
  }else if(value == HIGH){
    port |= (1 << pin); 
  }
}