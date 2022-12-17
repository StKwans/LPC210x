#include "pinconnect.h"
#include "Serial.h"

void PrintReg(const char* label, uint32_t addr) {
  Serial.print(label);
  Serial.print("(0x");
  Serial.print(addr,HEX,8);
  Serial.print(")=0x");
  Serial.println(*((volatile uint32_t*)(addr)),HEX,8);
}

void setup() {
  Serial.begin(38400);
  Serial.println("LPC2102 Kwan Firmware v0.00");
  Serial.println(__DATE__ " " __TIME__);
  PrintReg("PLLCON",0xE01F'C080);
  PrintReg("PLLCFG",0xE01F'C084);
  PrintReg("PLLSTAT",0xE01F'C088);
  Serial.println(SCB.PCLK());
  GPIODriver::direct_blink();
}

void loop() {
  digitalWrite(13,true);
  for(volatile int i=0;i<1'000'000;i++) {}
  digitalWrite(13,false);
  for(volatile int i=0;i<1'000'000;i++) {}
}
