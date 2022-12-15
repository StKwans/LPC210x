#ifndef gpio_h
#define gpio_h

#include "scb.h"

class GPIODriver {
  /** Notes:
   * Pin mode is controlled by the Pin Connect Block, with driver in pinconnect.h . However,
   * in the LPC210x, without exception the reset state of each pin is GPIO. Therefore we don't
   * try to control the pin mode here, and rely on other drivers to set and reset the mode
   * when they go in and out of scope.
   *
   * The fast IO block can do more complicated things, like:
   *  * Set a mask so that only certain bits are affected by further operations
   *  * Access multiple pins simultaneously, for either read or write
   *  * Access the registers in byte or halfword mode
   * Upon demand, this driver will add features to support such hardware features.
   * Until then, we use the Arduino model of pinMode()/digitalRead()/digitalWrite().
   * Ambient functions are provided that delegate to an ambient object of this class.
   */
private:
  // Even though SCS is technically in the system control block, it only has one bit used
  // and that one bit is solely used to control legacy/fast GPIO support. So, we will declare
  // and use that register here, instead of in the SCB.
  static volatile uint32_t& SCS()     {return (*(volatile uint32_t*)(0xE01F'C1A0));}
  // Everything will use the fast GPIO from here on.
  static const uint32_t FIO_BASE_ADDR=0x3FFF'C000;
  static volatile uint32_t& FIODIR()  {return (*(volatile uint32_t*)(FIO_BASE_ADDR + 0x00));}
  static volatile uint32_t& FIOMASK() {return (*(volatile uint32_t*)(FIO_BASE_ADDR + 0x10));}
  static volatile uint32_t& FIOPIN()  {return (*(volatile uint32_t*)(FIO_BASE_ADDR + 0x14));}
  static volatile uint32_t& FIOSET()  {return (*(volatile uint32_t*)(FIO_BASE_ADDR + 0x18));}
  static volatile uint32_t& FIOCLR()  {return (*(volatile uint32_t*)(FIO_BASE_ADDR + 0x1C));}
public:
  static void direct_blink() {
    SCS()=1;
    FIODIR()=(1<<13);
    for(;;) {
      for(int i=0;i<1'000'000;i++) FIOSET()=(1<<13);
      for(int i=0;i<1'000'000;i++) FIOCLR()=(1<<13);
    }
  }
  GPIODriver() {
    SCS()=1;
  }
  void pinMode(int pinNumber, bool output) {
    if(output) {
      FIODIR()|= (1 << pinNumber); //Set the selected bit, leave the others alone
    } else {
      FIODIR()&=~ (1 << pinNumber); //Clear the selected bit, leave the others alone
    }
  }
  void digitalWrite(int pinNumber, bool high) {
    if(high) {
      FIOSET()=(1<<pinNumber);
    } else {
      FIOCLR()=(1<<pinNumber);
    }
  }
  bool digitalRead(int pinNumber) {
    return (FIOPIN()>>pinNumber) & 1;
  }
};

inline GPIODriver GPIO;
// Arduino uses ambient functions. Just delegate these to the GPIO driver.
inline void pinMode(int pinNumber, bool output) {GPIO.pinMode(pinNumber,output);}
inline bool digitalRead(int pinNumber) {return GPIO.digitalRead(pinNumber);}
inline void digitalWrite(int pinNumber, bool high) {GPIO.digitalWrite(pinNumber,high);}

#endif

