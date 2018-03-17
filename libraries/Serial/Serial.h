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
#include "LPC214x.h"

#ifdef SERIAL_IRQ
#include "Circular.h"
#endif

class HardwareSerial: public Stream {
  private:
#ifdef SERIAL_IRQ
    char rxbuf[128];
    Circular rx(rxbuf,128);
#endif
  public:
    unsigned int port;
    HardwareSerial(int Lport):port(Lport) {};
    void begin(unsigned int baud);
    void end();
    void listen();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual void write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
};

inline int HardwareSerial::available(void) {
  return ((ULSR(port) & 0x01)>0)?1:0;
}

inline int HardwareSerial::read(void) {
  if(available()) return URBR(port);
  return -1;
}

inline void HardwareSerial::flush(void) {
  UFCR(port)|=0x07;
}

inline void HardwareSerial::write(uint8_t c) {
  while (!(ULSR(port) & 0x20));
  UTHR(port) = c;
}

inline int HardwareSerial::peek(void) {
  return -1;
}


extern HardwareSerial Serial;
extern HardwareSerial Serial1;

#endif
