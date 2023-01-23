#include "pinconnect.h"
#include "Serial.h"
#include "timer.h"
#include "dump.h"

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

Hd dump(Serial);
extern const char rom_start[];
extern const char rom_end[];
extern const char iap_start[];
extern const char iap_end[];
extern const char ram_start[];
extern const char ram_end[];
const int gpio_light=13;
const int drdy=8;
void setup() {
  Serial.begin(230400);
  Serial.println("LPC2102 Kwan Firmware v0.00");
  Serial.println(__DATE__ " " __TIME__);
  PrintReg("PLLCON",0xE01F'C080);
  PrintReg("PLLCFG",0xE01F'C084);
  PrintReg("PLLSTAT",0xE01F'C088);
  PrintNum("PLL lock count",SCB.pll_lock_count());
  Serial.println(SCB.PCLK());
  /*
  Serial.print(rom_end-rom_start,DEC);
  Serial.println("B flash:");
  dump.region(rom_start,rom_end-rom_start);
  Serial.print(iap_end-iap_start,DEC);
  Serial.println("B Boot ROM:");
  dump.region(iap_start,iap_end-iap_start);
  Serial.print(ram_end-ram_start,DEC);
  Serial.println("B RAM:");
  dump.region(ram_start,ram_end-ram_start);
   */
  pinMode(gpio_light,true);
  pinMode(drdy,true);
  Timer1.set_capture(0);
}

uint32_t old_tcc=0;
uint32_t new_tcc=0;
uint32_t tc_quicktick=0;
bool quicktick_level=false;
bool light_level=false;
const uint32_t quicktick_delta=7'500'000;

void quicktick() {
  uint32_t qt_tc=Timer1.count();
  uint32_t delta=Timer1.delta(tc_quicktick,qt_tc);
  if(delta>quicktick_delta) {
    quicktick_level=!quicktick_level;
    digitalWrite(drdy,quicktick_level);
    tc_quicktick=qt_tc;
  }
}
     

void loop() {
  while(new_tcc==old_tcc) {
    new_tcc=Timer1.read_capture(0);
    quicktick();
  }
  light_level=!light_level;
  digitalWrite(gpio_light,light_level);
  Serial.print(light_level?"Tick count:  ":"  Tock count:");Serial.print(new_tcc,DEC,10);Serial.print(" (delta ");Serial.print(Timer1.delta(old_tcc,new_tcc),DEC,8);Serial.println(")");
  old_tcc=new_tcc;
}
