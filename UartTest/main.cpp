#include "timer.h"
#include "gpio.h"
#include "pinconnect.h"
#include "scb.h"
#include "Serial.h"

/* Universal Asynchronous Receiver Transmitter */
static const uint32_t UART0_BASE_ADDR	=0xE000'C000;
static const uint32_t UART1_BASE_ADDR	=0xE001'0000;
static const uint32_t UART_BASE_DELTA (UART1_BASE_ADDR-UART0_BASE_ADDR);
static volatile uint32_t  URBR(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x00));}
static volatile uint32_t& UTHR(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x00));}
static volatile uint32_t& UDLL(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x00));}
static volatile uint32_t& UDLM(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x04));}
static volatile uint32_t& UIER(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x04));}
static volatile uint32_t  UIIR(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x08));}
static volatile uint32_t& UFCR(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x08));}
static volatile uint32_t& ULCR(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x0C));}
static volatile uint32_t& UMCR(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x10));}
static volatile uint32_t  ULSR(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x14));}
static volatile uint32_t& UMSR(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x18));}
static volatile uint32_t& USCR(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x1C));}
static volatile uint32_t& UACR(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x20));}
static volatile uint32_t& UFDR(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x28));}
static volatile uint32_t& UTER(int port) {return (*(volatile uint32_t*)(UART0_BASE_ADDR+(port)*UART_BASE_DELTA + 0x30));}

void write(uint8_t c) {
  while (!(ULSR(0) & 0x20)) {}
  UTHR(0) = c;
};

void setup() {
  PinConnect.set_pin(0,1); //TX0
  PinConnect.set_pin(1,1); //RX0
  unsigned int baud=38400*5;
  ULCR(0) = 0x83;   // 8 bits, no parity, 1 stop bit, DLAB = 1
  //DLAB - Divisor Latch Access bit. When set, a certain memory address
  //       maps to the divisor latches, which control the baud rate. When
  //       cleared, those same addresses correspond to the processor end
  //       of the FIFOs. In other words, set the DLAB to change the baud
  //       rate, and clear it to use the FIFOs.

  unsigned int Denom=SystemControlBlock::PCLK/baud;
  unsigned int UDL=Denom/16;

  UDLM(0)=(UDL >> 8) & 0xFF;
  UDLL(0)=(UDL >> 0) & 0xFF;

  UFCR(0) = 0xC7; //Enable and clear both FIFOs, Rx trigger level=14 chars
  UFDR(0) = 0x10; //Disable fractional divider register
  ULCR(0) = 0x03; //Turn of DLAB - FIFOs accessable
  UIER(0)=0;
  write(0xAA);
  write(0xAA);
  write(0x00);
  write(0xFF);
  write(0x00);
  write(0xAA);
  write(0xAA);
  write(0xAA);
  write('K');
  write('w');
  write('a');
  write('n');
  GPIODriver::direct_blink();
}

void loop() {
  digitalWrite(13,true);
  for(int i=0;i<1'000'000;i++) {}
  digitalWrite(13,false);
  for(int i=0;i<1'000'000;i++) {}
}
