/*
  HardwareSerial.h - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified 28 September 2010 by Mark Sproul
*/

#ifndef Serial_h
#define Serial_h

#include <inttypes.h>
#include "Stream.h"
#include "pinconnect.h"
#include "scb.h"

template<int port>
class HardwareSerial: public Stream {
private:
/* Universal Asynchronous Receiver Transmitter */
  static const uint32_t UART0_BASE_ADDR	=0xE000'C000;
  static const uint32_t UART1_BASE_ADDR	=0xE001'0000;
  static const uint32_t UART_BASE_DELTA =(UART1_BASE_ADDR-UART0_BASE_ADDR);
  static volatile uint32_t  URBR() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x00));}
  static volatile uint32_t& UTHR() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x00));}
  static volatile uint32_t& UDLL() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x00));}
  static volatile uint32_t& UDLM() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x04));}
  static volatile uint32_t& UIER() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x04));}
  static volatile uint32_t  UIIR() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x08));}
  static volatile uint32_t& UFCR() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x08));}
  static volatile uint32_t& ULCR() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x0C));}
  static volatile uint32_t& UMCR() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x10));}
  static volatile uint32_t  ULSR() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x14));}
  static volatile uint32_t& UMSR() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x18));}
  static volatile uint32_t& USCR() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x1C));}
  static volatile uint32_t& UACR() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x20));}
  static volatile uint32_t& UFDR() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x28));}
  static volatile uint32_t& UTER() {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x30));}
public:
  HardwareSerial() {};
  void begin(unsigned int baud) {
    //Set up the pins
    if(port==0) {
      PinConnect.set_pin(0,1); //TX0
      PinConnect.set_pin(1,1); //RX0
    } else {
      PinConnect.set_pin(8,1); //TX1
      PinConnect.set_pin(9,1); //RX1
    }
    ULCR() = 0x83;   // 8 bits, no parity, 1 stop bit, DLAB = 1
    //DLAB - Divisor Latch Access bit. When set, a certain memory address
    //       maps to the divisor latches, which control the baud rate. When
    //       cleared, those same addresses correspond to the processor end
    //       of the FIFOs. In other words, set the DLAB to change the baud
    //       rate, and clear it to use the FIFOs.

    unsigned int Denom=SCB.PCLK()/baud;
    unsigned int UDL=Denom/16;

    UDLM()=(UDL >> 8) & 0xFF;
    UDLL()=(UDL >> 0) & 0xFF;

    UFCR() = 0xC7; //Enable and clear both FIFOs, Rx trigger level=14 chars
    ULCR() = 0x03; //Turn of DLAB - FIFOs accessable
    UIER()=0;
  };
  void end() {
    //set the pins to read (high Z)
    if(port==0) {
      PinConnect.set_pin(0,0); //TX0->GPIO
      PinConnect.set_pin(1,0); //RX0->GPIO
    } else {
      PinConnect.set_pin(8,0); //TX1->GPIO
      PinConnect.set_pin(9,0); //RX1->GPIO
    }
  };
  void listen() {};
  int available(void) override {
    int result=((ULSR() & 0x01)>0)?1:0;
    return result;
  };
  int peek(void) override {
    //Peek not supported for a pure hardware read
    return -1;
  };
  int read(void) override {
    if(available()>0) {
      return URBR();
    } else {
      return -1;
    }
  }
  void flush(void) override {
    UFCR()|=0x07;
  }
  void write(uint8_t c) override {
    while (!(ULSR() & 0x20));
    UTHR() = c;
  };
  using Print::write; // pull in write(str) and write(buf, size) from Print
};

//These are predeclared so that everyone can use them, but must be defined manually.
inline HardwareSerial<0> Serial;
inline HardwareSerial<1> Serial1;

#endif
