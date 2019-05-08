#ifndef HMCSENSOR_H
#define HMCSENSOR_H

#include <inttypes.h>
#include "Wire.h"
#include "packet.h"

class HMC5883 {
  private:
    static const int ADDRESS=0x1E;  // I2C address of HMC5883L

    TwoWire& port;
    int8_t read(uint8_t address);
    int16_t read16(uint8_t address);
  public:
    HMC5883(TwoWire& Lport):port(Lport) {begin();};
    void begin();
    void whoami(char* id);
    /** Old read method. Note that values y and z are reversed in order, because this is
        the way the register map is, and I didn't know it until after 36.290. This maintains
        backward compatibility. */
    void read(int16_t& x, int16_t& z, int16_t& y);
    /** New read method. Improvements:
        * produce output in expected xyz order
        * capture status
        * read in a 7-byte burst instead of three 2-byte bursts */
    void read(int16_t& x, int16_t& y, int16_t& z, uint8_t& status);
    bool fillConfig(Packet& ccsds);
};

#endif
