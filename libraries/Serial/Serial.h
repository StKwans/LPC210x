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

template<int port>
class HardwareSerial: public Stream {
private:
public:
  HardwareSerial() {};
  void begin(unsigned int baud) {
    //Set up the pins
    if(port==0) {
      set_pin(0,1); //TX0
      set_pin(1,1); //RX0
    } else {
      set_pin(8,1); //TX1
      set_pin(9,1); //RX1
    }
    ULCR(port) = 0x83;   // 8 bits, no parity, 1 stop bit, DLAB = 1
    //DLAB - Divisor Latch Access bit. When set, a certain memory address
    //       maps to the divisor latches, which control the baud rate. When
    //       cleared, those same addresses correspond to the processor end
    //       of the FIFOs. In other words, set the DLAB to change the baud
    //       rate, and clear it to use the FIFOs.

    unsigned int Denom=Time::PCLK/baud;
    unsigned int UDL=Denom/16;

    UDLM(port)=(UDL >> 8) & 0xFF;
    UDLL(port)=(UDL >> 0) & 0xFF;

    UFCR(port) = 0xC7; //Enable and clear both FIFOs, Rx trigger level=14 chars
    ULCR(port) = 0x03; //Turn of DLAB - FIFOs accessable
    UIER(port)=0;
  };
  void end() void HardwareSerial::end() {
    //set the pins to read (high Z)
    if(port==0) {
      set_pin(0,0); //TX0->GPIO
      set_pin(1,0); //RX0->GPIO
      gpio_set_read(0);
      gpio_set_read(1);
    } else {
      set_pin(8,0); //TX1->GPIO
      set_pin(9,0); //RX1->GPIO
      gpio_set_read(8);
      gpio_set_read(9);
    }
  };
  void listen() {};
  int available(void) override {
    int result=((ULSR(port) & 0x01)>0)?1:0;
    return result;
  };
  int peek(void) override {
    //Peek not supported for a pure hardware read
    return -1;
  };
  int read(void) override {
    if(available()>0) {
      return URBR(port);
    } else {
      return -1;
    }
  }
  void flush(void) override {
    UFCR(port)|=0x07;
  }
  void write(uint8_t c) override {
    while (!(ULSR(port) & 0x20));
    UTHR(port) = c;
  };
  using Print::write; // pull in write(str) and write(buf, size) from Print
};

//These are predeclared so that everyone can use them, but must be defined manually.
inline HardwareSerial<0> Serial;
inline HardwareSerial<1> Serial1;

#endif
