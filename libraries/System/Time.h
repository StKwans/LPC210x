#ifndef TIME_H
#define TIME_H

#include <inttypes.h>

#ifndef PLL0_MULTIPLIER
#define PLL0_MULTIPLIER 5
#endif
#ifndef DEFAULT_MAMTIM
#define DEFAULT_MAMTIM 4
#endif



void delay(unsigned int ms);

class Time {
private:
  static void measurePCLK();
public:
  static const int FOSC=12'000'000;
  static unsigned int PCLK,CCLK,timerInterval;
  static const unsigned int timerSec=60;
  Time(int pll0_M=PLL0_MULTIPLIER);
  static void setup_pll(unsigned int channel, unsigned int M);
  static void set_rtc(int y, int m, int d, int h, int n, int s);
  static uint32_t uptime();
  static inline uint32_t delta(uint32_t a, uint32_t b, uint32_t limit=0) {
    //Given a time a and b on a clock that rolls over in limit, calculate the 
    //number of ticks between them taking into account the fact that the clock can 
    //roll over and therefore that b can be less than a. For now, ignore the fact
    //that there are really two rollovers (ticks in 1 minute and 32-bit limit)
    //and consider that we will never hit the 32-bit limit.
    if(limit==0) limit=timerSec*PCLK;
    uint32_t result;
    if(b<a) {
      result=limit+b-a;
    } else {
      result=b-a;
    }
    return result;
  };
};


extern Time sysclock;
#endif

