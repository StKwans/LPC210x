#include "gpio.h"

void setup() {
  GPIODriver::direct_blink();
  pinMode(13,true);
}

void loop() {
  digitalWrite(13,true);
  for(int i=0;i<1'000'000;i++) {}
  digitalWrite(13,false);
  for(int i=0;i<1'000'000;i++) {}
}