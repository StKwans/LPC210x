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
};

extern Time sysclock;
#endif

