//
// Created by jeppesen on 12/6/22.
//

#ifndef pinconnect_h
#define pinconnect_h

#include <cinttypes>
#include <cstddef>

class PinConnectDriver {
private:
  static const uint32_t PINSEL_BASE_ADDR=0xE002'C000;
  static volatile uint32_t& PINSEL0() {return (*(volatile uint32_t*)(PINSEL_BASE_ADDR + 0x00));}
  static volatile uint32_t& PINSEL1() {return (*(volatile uint32_t*)(PINSEL_BASE_ADDR + 0x04));}
public:
  void set_pin(int pin, int mode) {
    int mask=~(0x3 << ((pin & 0x0F)<<1));
    int val=mode << ((pin & 0x0F)<<1);
    if(pin>=16) {
      PINSEL1()=(PINSEL1() & mask) | val;
    } else {
      PINSEL0()=(PINSEL0() & mask) | val;
    }
  }
};

inline PinConnectDriver PinConnect;

#endif
