#include <string.h>
#include "Serial.h"
#include "DirectTask.h"
#include "gpio.h"
#include "LPC214x.h"

const uint32_t fastReadPeriodMs=3;
const uint32_t slowReadPeriodMs=10*fastReadPeriodMs;
uint32_t readPeriodMs=fastReadPeriodMs; //Read period in ms

static const char version_string[]="Rocketometer v2.0 using Kwan FAT/SD library " __DATE__ " " __TIME__;

volatile bool collectData=false;

void collectDataTop(void* stuff) {
  collectData=true;
  directTaskManager.reschedule(1,readPeriodMs,0,collectDataTop,0); 
}

static const int IA=16807;
static const int IM=0x7FFFFFFF;
static const int IQ=127773;
static const int IR=2836;
static const int MASK=123459876;

int ran0(int Lseed=0xFFFFFFFF) {
  static int seed;
  if(Lseed!=0xFFFFFFFF) seed=Lseed;
  int k;
  seed^=MASK;
  k=seed/IQ;
  seed=IA*(seed-k*IQ)-IR*k;
  if(seed<0) seed+=IM;
  int result=seed;
  seed^=MASK;
  return result;
}

void collectDataBottom() {
  flicker();
  if(ran0()%100==0) {
    Serial.println();
    Serial.println("~");
    delay(10);
  }
}

void setup() {
  set_light(1,1);
  Serial.begin(9600);
  Serial.println(version_string);
  Serial.listen();

  directTaskManager.begin();
  directTaskManager.schedule(1,readPeriodMs,0,collectDataTop,0); 
  ran0(TTC(0));
}

void loop() {
  if(collectData) {
    collectData=false;
    collectDataBottom();
  }
  while(Serial.available()>0) {
    Serial.print((char)Serial.read());
  }   
}


