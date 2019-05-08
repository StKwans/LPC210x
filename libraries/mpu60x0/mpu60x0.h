//"I'll call you... MPU!"
//"MPU... What's that?"
//"It's like CPU, only neater!"
//--Edward Wong Hau Pepelu Tivruski IV and the CPU of the D-135 Artificial Satellite
//  Cowboy Bebop Session 9, "Jamming with Edward"

#ifndef MPU60X0_H
#define MPU60X0_H

#include <inttypes.h>
#include "Wire.h"
#include "packet.h"

class MPU60x0 {
protected:
  uint8_t ADDRESS;  // Address of MPU60x0. Will be the I2C address for an I2C part, or the P0.x number for the chip select of an SPI part
public:
  MPU60x0(int Laddress):ADDRESS(Laddress) {};
  virtual unsigned char read(unsigned char addr)=0;
  virtual void write(unsigned char addr, unsigned char data)=0;
  virtual int16_t read16(unsigned char addr) {return ((int16_t)read(addr))<<8 | ((int16_t)read(addr+1));};
  unsigned char whoami() {return read(0x75);};
  virtual bool read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t);
  bool begin(uint8_t gyro_scale=0, uint8_t acc_scale=0); //Do anything necessary to init the part. Bus is available at this point.
  bool fillConfig(Packet& ccsds);
};

//I2C version of MPU60x0
class MPU6050: public MPU60x0 {
private:
  TwoWire& port;
  static const char addr_msb=0x68; //ORed with low bit of a0 to get actual address
  int A0;
public:
  MPU6050(TwoWire& Lport,int LA0, uint8_t gyro_scale=0, uint8_t acc_scale=0):MPU60x0(addr_msb+LA0),port(Lport),A0(LA0) {begin(gyro_scale,acc_scale);};
  unsigned char read(unsigned char addr) override;
  void write(unsigned char addr, unsigned char data) override;
  int16_t read16(unsigned char addr) override;
  bool read(int16_t& ax, int16_t& ay, int16_t& az, int16_t& gx, int16_t& gy, int16_t& gz, int16_t& t) override;
};

//SPI version of MPU60x0. The 6000 supports both I2C and SPI,
//but use the 6050 class to access it as I2C even if it is
//a 6000.
class MPU6000:public MPU60x0 {

};

#endif
