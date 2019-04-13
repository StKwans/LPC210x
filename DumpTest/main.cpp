#include "Serial.h"
#include "dump.h"

Hd hd(Serial);
extern char device_id[];

void setup() {
  Serial.begin(9600);
  hd.dumpSource();
  hd.region(device_id,4096);
}

void loop() {

}
