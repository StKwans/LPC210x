/*
  HardwareSerial.cpp - Hardware serial library for Wiring
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
  
  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "Serial.h"
#include "Time.h"
#include "gpio.h"

#ifdef SERIAL_IRQ
#include "vic.h"
void uart0_isr(void) {
  //We have to read the interrupt identification register
  //to acknowledge and clear the interrupt
  int iir=UIIR(0); 
  //Now that we have read it, we don't care what the actual reason is, because
  //we only know how to handle read events
  if(((iir & 0x0F)==0x0C) || ((iir & 0x0F)==0x04)) {
    //handle the read event
    int charcount=0;
    while((ULSR(0) & 1)>0) {
      charcount++;
      Serial.rx.fill(URBR(0));
    }
  }
  Serial.rx.mark();
}

void uart1_isr(void) {
  //We have to read the interrupt identification register
  //to acknowledge and clear the interrupt
  int iir=UIIR(1);
//  if(iir & 0x03<<1) {
    //handle the read event
    while((ULSR(1) & 1)>0) {
      Serial1.rx.fill(URBR(1));
    }
//  }
}
#endif



// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(unsigned int baud) {
  //Set up the pins
  if(port==0) {
    gpio_set_write(0);
    gpio_set_read(1);
    set_pin(0,1); //TX0
    set_pin(1,1); //RX0
  } else {
    gpio_set_write(8);
    gpio_set_read(9);
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
}

void HardwareSerial::end() {
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
#ifdef SERIAL_IRQ
  VIC.uninstall(VICDriver::UART0+port);
  UIER(port)=0x00; //Rx ready, not Tx empty, not line status, not autobaud ints
#endif
}

void HardwareSerial::listen(void) {
#ifdef SERIAL_IRQ
  VIC.install(VICDriver::UART0+port,port==0?uart0_isr:uart1_isr);
  UIER(port)=0x01; //Rx ready, not Tx empty, not line status, not autobaud ints
#endif
}

// Preinstantiate Objects //////////////////////////////////////////////////////

HardwareSerial Serial(0);
HardwareSerial Serial1(1);

volatile int f() {
  return 0x12345678;
}
int testInit=f();

