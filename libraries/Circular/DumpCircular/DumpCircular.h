#ifndef PrintCircular_h
#define PrintCircular_h

#include "dump.h"
#include "Circular.h"

class DumpCircular: public Circular {
private:
  static const int bufSize=256;
  char buf[bufSize];
protected:
  Dump& d;
public:
  unsigned int errno;
  DumpCircular(Dump& Ld):Circular(sizeof(buf),buf),d(Ld) {};

  virtual bool drain() override {
    if(tail<=mid) { //We don't have to go around the corner
      d.region(buf+tail,0,mid-tail,d.preferredLen); //Must specify record length, otherwise we get d.region(buf,len,rec_len) instead of d.region(buf,base,len) which doesn't exist
    } else {
      d.region(buf+tail,buf,0,bufSize-tail,mid);
    }
    tail=mid;
    return true;
  };
};

#endif
