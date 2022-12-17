#include "pinconnect.h"
#include "Serial.h"
#include "timer.h"

void PrintReg(const char* label, uint32_t addr) {
  Serial.print(label);
  Serial.print("(0x");
  Serial.print(addr,HEX,8);
  Serial.print(")=0x");
  Serial.println(*((volatile uint32_t*)(addr)),HEX,8);
}

void PrintNum(const char* label, uint32_t value) {
  Serial.print(label);
  Serial.print(": ");
  Serial.println(value,DEC,-10);
}

void setup() {
  Serial.begin(38400);
  Serial.println("LPC2102 Kwan Firmware v0.00");
  Serial.println(__DATE__ " " __TIME__);
  PrintReg("PLLCON",0xE01F'C080);
  PrintReg("PLLCFG",0xE01F'C084);
  PrintReg("PLLSTAT",0xE01F'C088);
  PrintNum("PLL lock count",SCB.pll_lock_count());
  Serial.println(SCB.PCLK());
  pinMode(13,true);
}

void loop() {
  digitalWrite(13,true);
  delay(1000);
  PrintNum("Tick count  ",Timer0.count());
  digitalWrite(13,false);
  delay(1000);
  PrintNum("  Tock count",Timer0.count());
}
