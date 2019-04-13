#include "Time.h"
#include "LPC214x.h"

unsigned int Time::PCLK;
unsigned int Time::CCLK;
unsigned int Time::timerInterval;

void Time::measurePCLK(void) {
  CCLK=FOSC*((PLLSTAT(0) & 0x1F)+1);
  switch (VPBDIV & 0x03) {
    case 0:
      PCLK=CCLK/4;
      break;
    case 1:
      PCLK=CCLK;
      break;
    case 2:
      PCLK=CCLK/2;
      break;
    case 3:
      break;
  }
}
                           //    J  F  M  A  M  J  J  A  S  O  N  D
static const char monthTable[]={ 0,31,28,31,30,31,30,31,31,30,31,30};

void Time::set_rtc(int y, int m, int d, int h, int n, int s) {
  RTCYEAR=y;
  RTCMONTH=m;
  RTCDOM=d;
  RTCDOY=monthTable[m]+d+((m>2) & ((y-2000)%4==0))?1:0;
  int DOC=RTCDOY-1+y/4+(y-2001)*365+1; //1 Jan 2001 was Monday
  RTCDOW=y>2000?(DOC % 7):0; //0-Sunday -- 6-Saturday
  RTCHOUR=h;
  RTCMIN=n;
  RTCSEC=s;
}

uint32_t Time::uptime() {
  return RTCDOY*86400+RTCHOUR*3600+RTCMIN*60+RTCSEC;
}

Time::Time(int pll0_M) {
  // Setting peripheral Clock (pclk) to System Clock (cclk)
  VPBDIV=0x1;
  setup_pll(0,pll0_M); //Set up CCLK PLL to 5x crystal rate
  // Enabling MAM and setting number of clocks used for Flash memory fetch (4 cclks in this case)
  //MAMTIM=0x3; //VCOM?
  MAMCR=0x2;
  MAMTIM=0x4; //Original

  //Set up Timer0 to count up to timerSec seconds at full speed, then auto reset with no interrupt.
  //This is needed for the accurate delay function and the task manager
  TTCR(0) = (1 << 1);       // Reset counter and prescaler and halt timer
  TCTCR(0) = 0; //Drive timer from PCLK, not external pin
  TMCR(0) = (1 <<1);     // On MR0, reset but no int.
  timerInterval=PCLK*timerSec-1;
  TMR0(0) = timerInterval;  //Reset when timer equals PCLK rate, effectively once per timerSec seconds
  TPR(0) = 0;  //No prescale, 1 timer tick equals 1 PCLK tick

  //Set up PWM timer to support analogWrite(). Ticks at 1MHz, resets every 1024 ticks. 
  PWMPR    = PCLK/1000000-1;      /* Load prescaler  */
  PWMPCR = (1 << 13);             /* Enable PWM5 and no others, single sided */
  PWMMCR = (1 <<  1);             /* On match with timer reset the counter   */
  PWMMR0 = 0x400;                 /* set cycle rate to 1024 ticks            */
  PWMMR5 = 0x200;                 /* set edge of PWM5 to 512 ticks           */
  PWMLER = (1 <<  0) | (1 << 5);  /* enable shadow latch for match 0 and 5   */
  PWMTCR = 0x00000002;            /* Reset counter and prescaler             */

  //Turn off the real-time clock
  CCR=0;
  //Set the PCLK prescaler and set the real-time clock to use it, so as to run in sync with everything else.
  PREINT=PCLK/32768-1;
  PREFRAC=PCLK-((PREINT+1)*32768);
  
  //If the clock year is reasonable, it must have been set by 
  //some process before, so we'll leave it running.
  //If it is year 0, then it is runtime from last reset, 
  //so we should reset it.
  //If it is unreasonable, this is the first time around,
  //and we set it to zero to count runtime from reset
  if(RTCYEAR<2000 || RTCYEAR>2100) {
    CCR|=(1<<1);
    set_rtc(0,0,0,0,0,0);
    //Pull the subsecond counter out of reset
    CCR&=~(1<<1);
  }
  
  //These are close together so that Timer0, RTC, and PWM are as near in-phase as possible
  TTCR(0) = (1 << 0); // start timer
  CCR|=(1<<0); //Turn the real-time clock on
  PWMTCR = 0x00000009;            /* enable counter and PWM, release counter from reset */
}

/**accurate delay. Relies on Timer0 running without pause at PCLK and resetting
 at timerSec seconds, as by Time::Time(). Code only reads, never writes, Timer0 registers */
void delay(unsigned int count) {
  unsigned int TC0=TTC(0);
  //count off whole timer reset cycles
  while(count>=1000*Time::timerSec) {
    //wait for the top of the timer reset cycle
    while(TTC(0)>TC0) ;
    //wait for the bottom of the timer reset cycle
    while(TTC(0)<TC0) ;
    count-=1000*Time::timerSec;
  }
  if(count==0) return;
  unsigned int TC1=TC0+count*(Time::PCLK/1000);
  if(TC1>Time::timerInterval) {
    //Do this while we are waiting
    TC1-=Time::timerInterval;
    //wait for the top of the timer reset cycle
    while(TTC(0)>TC0) ;
  }
  //wait for the rest of the delay time
  while(TTC(0)<TC1) ;
}

//The docs say that a successful feed must consist of two writes with no 
//intervening APB cycles. Use asm to make sure that it is done with two
//intervening instructions.
static void feed(int channel) {
  asm("mov r0, %0\n\t"
      "mov r1,#0xAA\n\t"
      "mov r2,#0x55\n\t"
      "str r1,[r0]\n\t"
      "str r2,[r0]\n\t" : :"r"(&PLLFEED(channel)):"r0","r1","r2");
//  PLLFEED(channel)=0xAA;
//  PLLFEED(channel)=0x55;
}

/** Set up on-board phase-lock-loop clock multiplier.

\param channel Channel 0 is the system PLL used to generate CCLK up to 60MHz.
               Channel 1 is the USB PLL used to generate its 48MHz.
\param M Clock multiplier. Final clock rate will be crystal frequency times this
number. May be between 1 and 32, but in practice must not exceed 5 with a 12MHz 
crystal.
*/	        
void Time::setup_pll(unsigned int channel, unsigned int M) {
  //Figure out N, exponent for PLL divider value, P=2^N. Intermediate frequency will be
  //FOSC*M*2*P=FOSC*M*2*2^N, and must be between 156MHz and 320MHz. This selects the lowest
  //N which satisfies the frequency constraint
  unsigned int N=0;
  while(FOSC*M*2*(1<<N)<156000000) N++;
  // Set Multiplier and Divider values
  PLLCFG(channel)=(M-1)|(N<<5);
  feed(channel);

  // Enable the PLL */
  PLLCON(channel)=0x1;
  feed(channel);

  // Wait for the PLL to lock to set frequency
  while(!(PLLSTAT(channel) & (1 << 10))) ;

  // Connect the PLL as the clock source
  PLLCON(channel)=0x3;
  feed(channel);
  //Make sure PCLK and CCLK variables are correct
  if(channel==0) measurePCLK();
}

Time sysclock;
                                              
