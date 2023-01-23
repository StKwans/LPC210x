#ifndef timer_h
#define timer_h

#include <cinttypes>
#include "scb.h"
#include "pinconnect.h"

template<int port>
class Timer32 {
private:
  static const uint32_t TMR0_BASE_ADDR = 0xE000'4000;
  static const uint32_t TMR1_BASE_ADDR = 0xE000'8000;
  static const uint32_t TMR_BASE_DELTA =(TMR1_BASE_ADDR-TMR0_BASE_ADDR);
protected:
  static volatile uint32_t& TIR()                 {return (*(volatile uint32_t*)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x00));}
  static volatile uint32_t& TTCR()                {return (*(volatile uint32_t*)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x04));}
  static volatile uint32_t& TTC()                 {return (*(volatile uint32_t*)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x08));}
  static volatile uint32_t& TPR()                 {return (*(volatile uint32_t*)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x0C));}
  static volatile uint32_t& TPC()                 {return (*(volatile uint32_t*)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x10));}
  static volatile uint32_t& TMCR()                {return (*(volatile uint32_t*)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x14));}
  static volatile uint32_t& TMR(uint32_t channel) {return (*(volatile uint32_t*)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x18+(channel)*4));}
  static volatile uint32_t& TCCR()                {return (*(volatile uint32_t*)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x28));}
  static volatile uint32_t& TCR(uint32_t channel) {return (*(volatile uint32_t*)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x2C+(channel)*4));}
  static volatile uint32_t& TEMR()                {return (*(volatile uint32_t*)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x3C));}
  static volatile uint32_t& TCTCR()               {return (*(volatile uint32_t*)(TMR0_BASE_ADDR+(port)*TMR_BASE_DELTA + 0x70));}
  static const constexpr uint8_t pin_map_p[8]={2,4,6,255,10,11,17,18};
  static const uint8_t pin_mode_timer=2;
  static const uint8_t pin_mode_gpio=0;
public:
  Timer32() {}
  static void stop_and_reset() {
    TTCR() = (1 << 1);       // Reset counter and prescaler and halt timer
  }
  static void start() {
    TTCR() = (1 << 0); // start timer
  }
  static uint32_t delta(uint32_t a, uint32_t b, uint32_t limit) {
    //Given a time a and b on a clock that rolls over in limit, calculate the
    //number of ticks between them taking into account the fact that the clock can
    //roll over and therefore that b can be less than a. For now, ignore the fact
    //that there are really two rollovers (ticks in 1 minute and 32-bit limit)
    //and consider that we will never hit the 32-bit limit.
    uint32_t result;
    if(b<a) {
      result=limit+b-a;
    } else {
      result=b-a;
    }
    return result;
  };
  static uint32_t count() {
    return TTC();
  }
  static void set_capture(uint32_t channel, bool rising=true, bool falling=false, bool intr=false, bool grab_pin=true) {
    if(grab_pin) {
      PinConnect.set_pin(pin_map_p[port * 4 + channel], pin_mode_timer);
    }
    uint32_t mask=7<<(channel*3);
    uint32_t value=((rising?1:0)|(falling?2:0)|(intr?4:0))<<(channel*3);
    TCCR()=(TCCR() &~mask)|value;
  }
  static void release_capture(uint32_t channel, bool release_pin=true) {
    uint32_t mask=7<<(channel*3);
    uint32_t value=0;
    TCCR()=(TCCR() &~mask)|value;
    if(release_pin) {
      PinConnect.set_pin(pin_map_p[port*4+channel],pin_mode_gpio);
    }
  }
  static uint32_t read_capture(uint32_t channel) {
    return TCR(channel);
  }

};

template<int port>
class KwanTimer:public Timer32<port> {
private:
  uint32_t timerSec;
  uint32_t timerInterval;
public:
  KwanTimer(uint32_t LtimerSec=60):Timer32<port>(),timerSec(LtimerSec) {
    //Set up Timer0 to count up to timerSec seconds at full speed, then auto reset with no interrupt.
    //This is needed for the accurate delay function and the task manager
    Timer32<port>::stop_and_reset();
    Timer32<port>::TCTCR() = 0; //Drive timer from PCLK, not external pin
    Timer32<port>::TMCR() = (1 << 1);     // On MR0, reset but no int.
    timerInterval = SCB.PCLK() * timerSec;
    Timer32<port>::TMR(0) = timerInterval-1;  //Reset when timer equals PCLK rate, effectively once per timerSec seconds
    Timer32<port>::TPR() = 0;  //No prescale, 1 timer tick equals 1 PCLK tick
    Timer32<port>::start();
  }
  uint32_t delta(uint32_t a, uint32_t b) {
    return Timer32<port>::delta(a,b,timerInterval);
  }
  /**accurate delay. Relies on Timer0 running without pause at PCLK and resetting
     at timerSec seconds, as by Time::Time(). Code only reads, never writes, Timer0 registers */
  void delay(uint32_t ms) {
    uint32_t TC0=Timer32<port>::TTC();
    //count off whole timer reset cycles
    while(ms>=1000*timerSec) {
      //wait for the top of the timer reset cycle
      while(Timer32<port>::TTC()>TC0) ;
      //wait for the bottom of the timer reset cycle
      while(Timer32<port>::TTC()<TC0) ;
      ms-=1000*timerSec;
    }
    if(ms==0) return;
    unsigned int TC1=TC0+ms*(SCB.PCLK()/1000);
    if(TC1>timerInterval) {
      //Do this while we are waiting
      TC1-=timerInterval;
      //wait for the top of the timer reset cycle
      while(Timer32<port>::TTC()>TC0) ;
    }
    //wait for the rest of the delay time
    while(Timer32<port>::TTC()<TC1) ;
  }

};

Timer32<0> Timer0;
KwanTimer<1> Timer1;

inline void delay(uint32_t ms) {
  Timer1.delay(ms);
}

#endif
