#include "mpu60x0.h"
#include "Serial.h"
#include "Time.h"

bool MPU60x0::fillConfig(Packet& ccsds) {
  if(!ccsds.fill(ADDRESS   ,"I2Caddress"  )) return false;
  if(!ccsds.fill(read(0x0D),"self_test_x" )) return false;
  if(!ccsds.fill(read(0x0E),"self_test_y" )) return false;
  if(!ccsds.fill(read(0x0F),"self_test_z" )) return false;
  if(!ccsds.fill(read(0x10),"self_test_a" )) return false;
  if(!ccsds.fill(read(0x19),"smplrt_div"  )) return false;
  if(!ccsds.fill(read(0x1A),"config"      )) return false;
  if(!ccsds.fill(read(0x1B),"gyro_config" )) return false;
  if(!ccsds.fill(read(0x1C),"accel_config")) return false;
  if(!ccsds.fill(read(0x1F),"mot_thr"     )) return false;
  if(!ccsds.fill(read(0x37),"int_pin_cfg" )) return false;
  if(!ccsds.fill(read(0x38),"int_enable"  )) return false; 
  if(!ccsds.fill(read(0x6B),"pwr_mgmt_1"  )) return false; 
  if(!ccsds.fill(read(0x75),"whoami"      )) return false;
  return true;
}

bool MPU60x0::begin(uint8_t gyro_scale, uint8_t acc_scale) {
  //Serial.print("MPU60x0::begin(");Serial.print(gyro_scale,DEC);Serial.print(",");Serial.print(acc_scale,DEC);Serial.println(")");
  //Wake the part up, Set up the clock source (x gyro, 1)
  write(0x6B,(0 << 7) | (0 << 6) | (0 << 5) | (0x01 << 0));
  delay(50);
  //Do anything necessary to init the part. Bus is available at this point.
  //Set External sync (none, 0x00) and Low Pass Filter (~40Hz bandwidth, 0x03)
  write(0x1A,(0x00<<3) | (0x03<<0));
  //Turn off all gyro self-test and set the gyro scale
  uint8_t gyro_config=(0 << 7) | (0 << 6) | (0 << 5) | ((gyro_scale & 0x03) << 3);
  //Serial.print("Gyro config: 0x");Serial.println(gyro_config,HEX,2);
  write(0x1B,gyro_config);
  //Turn off all acc  self-test and set the acc  scale
  uint8_t acc_config=(0 << 7) | (0 << 6) | (0 << 5) | ((acc_scale  & 0x03) << 3);
  //Serial.print("Acc  config: 0x");Serial.println(acc_config,HEX,2);
  write(0x1C,acc_config);
  return true;
}

// Read 1 byte from the sensor at 'address'
unsigned char MPU6050::read(uint8_t address) {
  port.beginTransmission(ADDRESS);
  port.write(address);
  port.endTransmission();
  
  port.requestFrom(ADDRESS, 1);
  return port.read();
}

// Read a 16-bit integer in big-endian format from the sensor
// First byte will be from 'address'
// Second byte will be from 'address'+1
int16_t MPU6050::read16(uint8_t address) {
  int16_t msb, lsb;
  
  port.beginTransmission(ADDRESS);
  port.write(address);
  port.endTransmission();
  
  port.requestFrom(ADDRESS, 2);
  msb = port.read();
  lsb = port.read();
  
  return msb<<8 | lsb;
}

void MPU6050::write(uint8_t address, uint8_t data) {
  port.beginTransmission(ADDRESS);
  port.write(address);
  port.write(data);
  port.endTransmission();
}

bool MPU60x0::read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t) {
  ax=read16(0x3B);
  ay=read16(0x3D);
  az=read16(0x3F);
  t =read16(0x41);
  gx=read16(0x43);
  gy=read16(0x45);
  gz=read16(0x47);
  return true;
}

bool MPU6050::read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t) {
  int msb, lsb;
  port.beginTransmission(ADDRESS);
  port.write(0x3B);
  port.endTransmission();
  
  port.requestFrom(ADDRESS, 14);
  msb = port.read();
  lsb = port.read();
  ax= msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  ay= msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  az= msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  t= msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  gx= msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  gy= msb<<8 | lsb;
  msb = port.read();
  lsb = port.read();
  gz= msb<<8 | lsb;
  return true;
}



