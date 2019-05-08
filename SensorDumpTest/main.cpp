#include "Serial.h"
#include "DumpCircular.h"
#include "packet.h"
#include "Time.h"
#include "StateTwoWire.h"
#include "bmp180.h"
#include "mpu60x0.h"
#include "hmc5883.h"

HardwareSerial Serial(0,115200);
StateTwoWire I2C0(0);
BMP180 bmp(I2C0,3);
MPU6050 mpu(I2C0,0);
HMC5883 hmc(I2C0);
Hd hd(Serial,32);
DumpCircular d(hd);
uint16_t seq[16];
bool docd[16];
//Put other zeroed variables above this one. Whatever is on top of .bss is more
//likely to be overwritten by the stack, and this one is kind of a temporary anyway.
char stashbuf[128]; 
CCSDS packet(d,seq,docd,stashbuf);

void setup() {
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

  packet.start(0x05,"BMP180calibration");
  packet.fill(bmp.whoami(),"whoami_should_be_0x55");
  bmp.fillCalibration(packet);
  packet.finish(0x05);

  packet.start(0x07,"MPU6050config");
  mpu.fillConfig(packet);
  packet.finish(0x07);  

  packet.start(0x09,"HMC5883Lconfig");
  hmc.fillConfig(packet);
  packet.finish(0x09);  
}

void loop() {
  uint32_t tc0=TTC(0);
  bmp.takeMeasurement();
  uint32_t tc1=TTC(0);
  uint32_t dt=Time::delta(tc0,tc1);
  packet.start(0x06,"BMP180measurement",tc0);
  packet.fillu32(bmp.getTemperatureRaw(),"UT");
  packet.filli16(bmp.getTemperature(),"UT_cal");
  packet.fillu32(bmp.getPressureRaw(),"UP");
  packet.fillu32(bmp.getPressure(),"UP_cal");
  packet.fillu32(dt,"dt");
  packet.finish(0x06);

  int16_t ax,ay,az,bx,by,bz,gx,gy,gz,t;
  tc0=TTC(0);
  mpu.read(ax,ay,az,gx,gy,gz,t);
  tc1=TTC(0);
  dt=Time::delta(tc0,tc1);
  packet.start(0x08,"MPU6050measurement",tc0);
  packet.filli16(ax,"ax");
  packet.filli16(ay,"ay");
  packet.filli16(az,"az");
  packet.filli16(gx,"gx");
  packet.filli16(gy,"gy");
  packet.filli16(gz,"gz");
  packet.filli16(t ,"t" );
  packet.fillu32(dt,"dt");
  packet.finish(0x08);

  uint8_t status;
  tc0=TTC(0);
  hmc.read(bx,by,bz,status);
  tc1=TTC(0);
  dt=Time::delta(tc0,tc1);
  packet.start(0x0A,"HMC5883Lmeasurement",tc0);
  packet.filli16(bx,"bx");
  packet.filli16(by,"by");
  packet.filli16(bz,"bz");
  packet.fill(status,"status");
  packet.fillu32(dt,"dt");
  packet.finish(0x0A);
  
  delay(1000);
}

