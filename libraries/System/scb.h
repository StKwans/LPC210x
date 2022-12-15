#ifndef TIME_H
#define TIME_H

#include <cinttypes>

#ifndef FOSC
#define FOSC 12'000'000
#endif

//Leave the following as preprocessor symbols so they can be defined at the command line
#ifndef PLL_MULTIPLIER
#define PLL_MULTIPLIER 5
#endif
#ifndef DEFAULT_MAMTIM
#define DEFAULT_MAMTIM 4
#endif


class SystemControlBlock {
  static const uint32_t SCB_BASE_ADDR=0xE01F'C000;

  class PLLDriver {
  private:
    /* Phase Locked Loop (PLL) */
    static volatile uint32_t& PLLCON() {return (*(volatile uint32_t*)(SCB_BASE_ADDR + 0x80));}
    static volatile uint32_t& PLLCFG() {return (*(volatile uint32_t*)(SCB_BASE_ADDR + 0x84));}
    static volatile uint32_t& PLLSTAT(){return (*(volatile uint32_t*)(SCB_BASE_ADDR + 0x88));}
    static volatile uint32_t& PLLFEED(){return (*(volatile uint32_t*)(SCB_BASE_ADDR + 0x8C));}
    //The docs say that a successful feed must consist of two writes with no
    //intervening APB cycles. Use asm to make sure that it is done with two
    //intervening instructions.
    static void feed() {
      asm("mov r0, %0\n\t"
          "mov r1,#0xAA\n\t"
          "mov r2,#0x55\n\t"
          "str r1,[r0]\n\t"
          "str r2,[r0]\n\t" : :"r"(&PLLFEED()):"r0","r1","r2");
      //  PLLFEED(channel)=0xAA;
      //  PLLFEED(channel)=0x55;
    }

  public:
    /** Set up on-board phase-lock-loop clock multiplier.
    \param M Clock multiplier. Final clock rate will be crystal frequency times this
    number. May be between 1 and 32, but in practice must not exceed 5 with a 12MHz
    crystal.
    */
    static const uint32_t FCCOmin=156'000'000;
    static const uint32_t FCCOmax=320'000'000;
    static void init_pll() {
      //Figure out N, exponent for PLL divider value, P=2^N. Intermediate frequency FCCO will be
      //FOSC*M*2*P=FOSC*M*2*2^N, and must be between 156MHz and 320MHz. This selects the lowest
      //N which satisfies the frequency constraint

      unsigned int N=1;
      //while(FOSC*PLL_MULTIPLIER*2*(1<<N)<FCCOmin) N++;
      // Set Multiplier and Divider values
      PLLCFG()=(PLL_MULTIPLIER-1)|(N<<5);
      feed();

      // Enable the PLL */
      PLLCON()=0x1;
      feed();

      // Wait for the PLL to lock to set frequency
      while(!locked()) {}

      GPIODriver::direct_blink();
      // Connect the PLL as the clock source
      PLLCON()=0x3;
      feed();
    }
    PLLDriver() {
      init_pll();
    }
    uint32_t multiplier() {return (PLLSTAT() & 0x1F)+1;}
    uint32_t divider()    {return 1<<((PLLSTAT() >> 5) & 0x03);}
    bool enabled()        {return (PLLSTAT() >> 8) & 0x01;}
    bool connected()      {return (PLLSTAT() >> 9) & 0x01;}
    bool locked()         {return (PLLSTAT() >>10) & 0x01;}
    uint32_t FCCO()       {return FOSC*2*multiplier()*divider();}
  };

  static PLLDriver PLL;
private:
  static void measurePCLK() {
    CCLK=FOSC*PLL.multiplier();
    switch (VPBDIV() & 0x03) {
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
  /* Memory Accelerator Module (MAM) */
  static volatile uint32_t& MAMCR()  {return (*(volatile uint32_t*)(SCB_BASE_ADDR + 0x000));}
  static volatile uint32_t& MAMTIM() {return (*(volatile uint32_t*)(SCB_BASE_ADDR + 0x004));}
  static volatile uint32_t& MEMMAP() {return (*(volatile uint32_t*)(SCB_BASE_ADDR + 0x040));}

  /* VPB Divider (CCLK is divided by this to get PCLK) */
  static volatile uint32_t& VPBDIV() {return (*(volatile uint32_t*)(SCB_BASE_ADDR + 0x100));}

public:
  static uint32_t PCLK,CCLK,timerInterval;
  SystemControlBlock() {
    // Setting peripheral Clock (pclk) to System Clock (cclk)
    VPBDIV()=0x1;
    // Enabling MAM and setting number of clocks used for Flash memory fetch (4 cclks in this case)
    //MAMTIM=0x3; //VCOM?
    MAMCR()=0x2;
    MAMTIM()=0x4; //Original

    //Make sure PCLK and CCLK variables are correct
    measurePCLK();
  }
};


inline uint32_t SystemControlBlock::PCLK;
inline uint32_t SystemControlBlock::CCLK;
inline SystemControlBlock::PLLDriver SystemControlBlock::PLL;
inline SystemControlBlock SCB;
#endif

