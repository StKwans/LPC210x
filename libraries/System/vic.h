#ifndef VIC_H 
#define VIC_H

#include "irq.h"

extern "C" {
void IRQ_Wrapper();
}

class VICDriver {
private:
  static const uint32_t VIC_BASE_ADDR=0xFFFFF000;

  static volatile uint32_t& VICIRQStatus()  {return (*(volatile uint32_t*)(VIC_BASE_ADDR + 0x000));}
  static volatile uint32_t& VICFIQStatus()  {return (*(volatile uint32_t*)(VIC_BASE_ADDR + 0x004));}
  static volatile uint32_t& VICRawIntr()    {return (*(volatile uint32_t*)(VIC_BASE_ADDR + 0x008));}
  static volatile uint32_t& VICIntSelect()  {return (*(volatile uint32_t*)(VIC_BASE_ADDR + 0x00C));}
  static volatile uint32_t& VICIntEnable()  {return (*(volatile uint32_t*)(VIC_BASE_ADDR + 0x010));}
  static volatile uint32_t& VICIntEnClr()   {return (*(volatile uint32_t*)(VIC_BASE_ADDR + 0x014));}
  static volatile uint32_t& VICSoftInt()    {return (*(volatile uint32_t*)(VIC_BASE_ADDR + 0x018));}
  static volatile uint32_t& VICSoftIntClr() {return (*(volatile uint32_t*)(VIC_BASE_ADDR + 0x01C));}
  static volatile uint32_t& VICProtection() {return (*(volatile uint32_t*)(VIC_BASE_ADDR + 0x020));}
  static volatile fvoid&    VICVectAddr()   {return (*(volatile fvoid*)(VIC_BASE_ADDR + 0x030));}
  static volatile fvoid&    VICDefVectAddr(){return (*(volatile fvoid*)(VIC_BASE_ADDR + 0x034));}
  static volatile fvoid&    VICVectAddrSlot(uint32_t slot) {return (*(volatile fvoid*)(VIC_BASE_ADDR + 0x100+((slot)*4)));}
  static volatile uint32_t& VICVectCntlSlot(uint32_t slot) {return (*(volatile uint32_t*)(VIC_BASE_ADDR + 0x200+((slot)*4)));}
  static void DefaultVICHandler(void) {
  /** 
    Default interrupt handler, called if no handler is installed for a particular interrupt.
    If the IRQ is not installed into the VIC, and interrupt occurs, the
    default interrupt VIC address will be used. This could happen in a race
    condition. For debugging, use this endless loop to trace back. 
    For more details, see Philips appnote AN10414 */
    //Print 'E' forever on UART0 at whatever is its current settings
    while ( 1 ) {
      //while (!(U0LSR & 0x20));
      //U0THR = 'E';
    }
  }

  static const int IRQ_SLOT_EN=(1 << 5); ///< bit 5 in Vector control register 
  static const int VIC_SIZE	=16; ///<Number of VIC slots
public:  
  /** Initialize the interrupt controller. Clear out all vector slots */
  VICDriver() {
    // initialize VIC
    VICIntEnClr() = 0xffffffff;
    VICVectAddr() = 0;
    VICIntSelect() = 0;

    // set all the vector and vector control register to 0 
    for (int i = 0; i < VIC_SIZE; i++ ) {
      VICVectAddrSlot(i) = nullptr;
      VICVectCntlSlot(i) = 0;
    }

    /* Install the default VIC handler here */
    VICDefVectAddr() = DefaultVICHandler;

    //Now it is safe to activate interrupts
    enable_ints();
  }

  /**
    Install an interrupt handler in the VIC. This finds an empty slot,
    installs the handler there, and associates it with the correct source channel.
  \note  The VIC slots are associated with priority, lower number is earlier priority.
     This handler installs the handler in the lowest numbered slot available, so
     if you care about priority, then when installing multiple handlers, install them
     first priority first, and so on.


   \param IntNumber Interrupt source channel
   \param HandlerAddr interrupt handler address
   \return      true if handler installed, false if not (table full)
  */
  bool install(unsigned int IntNumber, fvoid HandlerAddr) {
    VICIntEnClr() = 1 << IntNumber;   //Disable Interrupt 

    for (int i = 0; i < VIC_SIZE; i++ ) {
      // find first un-assigned VIC address for the handler 

      if ( VICVectAddrSlot(i) == nullptr ) {
        VICVectAddrSlot(i) = HandlerAddr;    // set interrupt vector 
        VICVectCntlSlot(i) = (IRQ_SLOT_EN | IntNumber);
        VICIntEnable() |= 1 << IntNumber;  // Enable Interrupt 
        return true;
      }
    }    
  
    return false;        // fatal error, can't find empty vector slot 
  }

  /**
    Remove an interrupt handler in the VIC. This finds the slot associated with this
    source channel and clears it.,

   \param IntNumber Interrupt source channel
   \return      true if handler was uninstalled, false if not (no handler for this channel installed in the first place) 

   \note You can install the same channel in multiple VIC slots. It's not a good idea, but possible. If you do so, this
         will only uninstall the one with the earliest priority (lowest numbered slot).
  */
  bool uninstall(unsigned int IntNumber) {
    VICIntEnClr() = 1 << IntNumber;   /* Disable Interrupt */
  
    for (int i = 0; i < VIC_SIZE; i++ ) {
      //find first VIC address assigned to this channel
      if ( (VICVectCntlSlot(i) & 0x1f ) == IntNumber ) {
        VICVectAddrSlot(i) = nullptr;   // clear the VIC entry in the VIC table 
        VICVectCntlSlot(i) = 0;   // disable SLOT_EN bit and mark slot as available 
        return true;
      }
    }
    return false;        // fatal error, can't find interrupt number in vector slot 
  }
  friend void IRQ_Wrapper();
  static const int WDT		= 0; ///< Watchdog timer
//  static const int SWI		= 1; ///< Software interrupt, triggered by SWI x instruction
  static const int ARM_CORE0	= 2; ///< ARMCore0, used by EmbeddedICE RX
  static const int ARM_CORE1	= 3; ///< ARMCore1, Used by EmbeddedICE TX
  static const int TIMER0	= 4; ///< Timer 0 match or capture 
  static const int TIMER1	= 5; ///< Timer 1 match or capture
  static const int UART0	= 6; ///< UART 0 interrupt
  static const int UART1	= 7; ///< UART 1 interrupt
//  static const int PWM0		= 8; ///< PWM match
  static const int I2C0		= 9; ///< I2C 0 interrupt
  static const int SPI0		=10; ///< SPI 0 interrupt
  static const int SPI1		=11; ///< SPI 1 interrupt
  static const int PLL		=12; ///< Phase lock loop in lock
  static const int RTC		=13; ///< Real-time clock increment or alarm
  static const int EINT0	=14; ///< External interrupt 0
  static const int EINT1	=15; ///< External interrupt 1
  static const int EINT2	=16; ///< External interrupt 2
//  static const int EINT3	=17; ///< External interrupt 3
  static const int ADC0		=18; ///< Analog-to-Digital Converter 0 end of conversion
  static const int I2C1		=19; ///< I2C 1 interrupt
//  static const int BOD		=20; ///< Brownout detected
//  static const int ADC1		=21; ///< Analog-to-Digital Converter 1 end of conversion
//  static const int USB		=22; ///< USB and DMA interrupt
  static const int TIMER2	=26; ///< Timer 0 match or capture 
  static const int TIMER3	=27; ///< Timer 1 match or capture
};

inline VICDriver VIC;

// This isn't inline, so if vic.h is included in multiple translation units, there will
// be multiple versions of it. Declare it weak, so that in this case, we don't violate ODR.
// If it is inline, we can't get a pointer to it as is needed in the vector table.
void __attribute__ ((interrupt("IRQ"),weak)) IRQ_Wrapper() {
  // Now we see why we like high-level languages for IRQ handling. It says treat
  // the number as a pointer to a function and call it.
  VIC.VICVectAddr()();
  // ACK the VIC
  VIC.VICVectAddr()=nullptr;
};


#endif


