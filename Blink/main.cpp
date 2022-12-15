#include "timer.h"
#include "gpio.h"

void setup() {
  pinMode(13,true);
}

void loop() {
  digitalWrite(13,true);
  delay(1000);
  digitalWrite(13,false);
  delay(1000);
}