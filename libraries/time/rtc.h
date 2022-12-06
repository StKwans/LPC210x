#ifndef rtc_h
#define rtc_h

#include "scb.h"

class RTCDriver {
private:
/* Real Time Clock */
  static const uint32_t RTC_BASE_ADDR=0xE002'4000;
  static volatile uint32_t& ILR()     {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x00));}
  static volatile uint32_t& CTC()     {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x04));}
  static volatile uint32_t& CCR()     {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x08));}
  static volatile uint32_t& CIIR()    {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x0C));}
  static volatile uint32_t& AMR()     {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x10));}
  static volatile uint32_t& CTIME0()  {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x14));}
  static volatile uint32_t& CTIME1()  {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x18));}
  static volatile uint32_t& CTIME2()  {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x1C));}
  static volatile uint32_t& RTCSEC()  {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x20));}
  static volatile uint32_t& RTCMIN()  {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x24));}
  static volatile uint32_t& RTCHOUR() {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x28));}
  static volatile uint32_t& RTCDOM()  {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x2C));}
  static volatile uint32_t& RTCDOW()  {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x30));}
  static volatile uint32_t& RTCDOY()  {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x34));}
  static volatile uint32_t& RTCMONTH(){return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x38));}
  static volatile uint32_t& RTCYEAR() {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x3C));}
  static volatile uint32_t& ALSEC()   {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x60));}
  static volatile uint32_t& ALMIN()   {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x64));}
  static volatile uint32_t& ALHOUR()  {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x68));}
  static volatile uint32_t& ALDOM()   {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x6C));}
  static volatile uint32_t& ALDOW()   {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x70));}
  static volatile uint32_t& ALDOY()   {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x74));}
  static volatile uint32_t& ALMON()   {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x78));}
  static volatile uint32_t& ALYEAR()  {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x7C));}
  static volatile uint32_t& PREINT()  {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x80));}
  static volatile uint32_t& PREFRAC() {return (*(volatile uint32_t*)(RTC_BASE_ADDR + 0x84));}

  //    J   F   M   A   M   J   J   A   S   O   N   D
  static constexpr uint32_t monthTable[12]={  0, 31, 59, 90,120,151,181,212,243,273,304,334};
public:
  RTCDriver() {
    //If the clock year is reasonable, it must have been set by
    //some process before, so we'll leave it running.
    //If it is year 0, then it is runtime from last reset,
    //so we should reset it.
    //If it is unreasonable, this is the first time around,
    //and we set it to zero to count runtime from reset
    //Turn off the real-time clock
    stop_and_reset();
    //Set the PCLK prescaler and set the real-time clock to use it, so as to run in sync with everything else.
    PREINT()=SCB.PCLK/32768-1;
    PREFRAC()=SCB.PCLK-((PREINT()+1)*32768);

    //These are close together so that Timer0, RTC, and PWM are as near in-phase as possible
    CCR()|=(1<<0); //Turn the real-time clock on
    if (RTCYEAR() < 2000 || RTCYEAR() > 2100) {
      CCR() |= (1 << 1);
      set(0, 0, 0, 0, 0, 0);
      //Pull the subsecond counter out of reset
      CCR() &= ~(1 << 1);
    }
    start();
  }
  void set(int y, int m, int d, int h, int n, int s) {
    RTCYEAR()=y;
    RTCMONTH()=m;
    RTCDOM()=d;
    int doy=monthTable[m-1]+d+(((m>2) & ((y-2000)%4==0))?1:0);
    RTCDOY()=doy;
    int doc=doy-1+y/4+(y-2001)*365+1; //1 Jan 2001 was Monday
    RTCDOW()=y>2000?(doc % 7):0; //0-Sunday -- 6-Saturday
    RTCHOUR()=h;
    RTCMIN()=n;
    RTCSEC()=s;
  }
  uint32_t uptime() {
    return RTCDOY()*86400+RTCHOUR()*3600+RTCMIN()*60+RTCSEC();
  }
  void stop_and_reset() {
    CCR()=0;
  }
  void start() {
    CCR()|=(1<<0);
  }
};



#endif
