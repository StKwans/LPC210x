#include "Serial.h"
#include "DumpCircular.h"
#include "packet.h"
#include "Time.h"

Hd hd(Serial,32);
DumpCircular d(hd);
uint16_t seq[16];
bool docd[16];
//Put other zeroed variables above this one. Whatever is on top of .bss is more
//likely to be overwritten by the stack, and this one is kind of a temporary anyway.
char stashbuf[128]; 
CCSDS packet(d,seq,docd,stashbuf);

void setup() {
  Serial.begin(115200);
  packet.metaDoc();
//Dump code to serial port and packet file
  int len=source_end-source_start;
  char* base=source_start;
  const int dumpPktSize=sizeof(stashbuf)-6-sizeof(uint32_t);
  while(len>0) {
    packet.start(0x04,"Source tarball");
    packet.fillu32(base-source_start,"offset");
    packet.fill(base,len>dumpPktSize?dumpPktSize:len,"tarball data");
    packet.finish(0x04);
//    Serial.print(".");
    base+=dumpPktSize;
    len-=dumpPktSize;
  }
}

void loop() {
  packet.start(0x03,"Test packet",TTC(0)); 
  packet.fillu32(PLLSTAT(0),"PLL Status Register"); 
  packet.fillu32(Time::FOSC,"Oscillator frequency");
  packet.fillu32(Time::CCLK,"Core clock frequency");
  packet.fillu32(Time::PCLK,"Peripheral clock frequency");
  packet.fillu32(RTCSEC,"RTC second count");
  packet.fillu32(CTC,"RTC subsecond count");
  packet.fillu32(sizeof(bool),"sizeof(bool)");
  packet.fillu32(true,"true constant");
  packet.fillu32(false,"false constant");
  packet.finish(0x03);
  delay(1000);
}

