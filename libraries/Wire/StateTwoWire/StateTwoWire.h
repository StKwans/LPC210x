#ifndef HARDTWOWIRE_H
#define HARDTWOWIRE_H

#include "Wire.h"
#include "LPC214x.h"

class StateTwoWire:public TwoWire {
  private:
    int port;
    void twi_init(unsigned int freq) override;
    uint8_t address;
    const char* dataWrite;
    char* dataRead;
    uint8_t lengthWrite;
    uint8_t lengthRead;
    volatile bool done;
    uint8_t twi_readFrom(uint8_t Laddress, char* Ldata, uint8_t Llength) override;
    uint8_t twi_writeTo(uint8_t Laddress, const char* Ldata, uint8_t Llength, uint8_t wait) override;
    static const int AA   =(1 << 2);
    static const int SI   =(1 << 3);
    static const int STO  =(1 << 4);
    static const int STA  =(1 << 5);
    static const int EN =(1 << 6);
    static StateTwoWire *thisPtr[2];
    static void IntHandler0();
    static void IntHandler1();
    void wait_si() {while(!(I2CCONSET(port) & SI)) ;}
    void stateDriver();
    friend void stateIn(StateTwoWire*);
    friend void stateSl(StateTwoWire*);
    friend void state00(StateTwoWire*);
    friend void state08(StateTwoWire*);
    friend void state10(StateTwoWire*);
    friend void state18(StateTwoWire*);
    friend void state20(StateTwoWire*);
    friend void state28(StateTwoWire*);
    friend void state30(StateTwoWire*);
    friend void state38(StateTwoWire*);
    friend void state40(StateTwoWire*);
    friend void state48(StateTwoWire*);
    friend void state50(StateTwoWire*);
    friend void state58(StateTwoWire*);
  public:
    StateTwoWire(int Lport,int freq=I2CFREQ):TwoWire(),port(Lport) {thisPtr[port]=this;begin(freq);}

};

#endif
