#ifndef dump_H
#define dump_H

#include "Print.h"

extern char btext[],etext[];
extern char source_start[];
extern char source_end[];

class Dump {
protected:
  Print& out; 
public:
  int preferredLen;
  Dump(Print& Lout, int LpreferredLen):out(Lout),preferredLen(LpreferredLen) {};
  virtual void begin() {};
  virtual void end() {};
  virtual void line(const char* start, int base, int len)=0;
  virtual void line(const char* start0, const char* start1, int base, int len0, int len1) {
    line(start0,base,len0);
    line(start1,base+len0,len1);
  }
  void region(const char* start, int base, int len, int rec_len);
  void region(const char* start, int len, int rec_len) {region(start,(int)start,len,rec_len);};
  void region(const char* start, int len) {region(start,0,len,preferredLen);};
  //Display two regions as if they were contiguous. Useful for going around
  //the corner of a circular buffer
  void region(const char* start0, const char* start1, int base, int len0, int len1, int rec_len);
  void region(const char* start0, const char* start1, int base, int len0, int len1) {region(start0,start1,base,len0,len1,preferredLen);};
  //The following matches the above for printing a default base, but "default base"
  //doesn't make sense for two consecutive regions.
  //void region(const char* start0, const char* start1, int len0, int len1) {region(start0,start1,(int)start0,len0,len1,preferredLen);};
  void dumpText() {region(btext,etext-btext,preferredLen);};
  void dumpSource() {region(source_start,0,source_end-source_start,preferredLen);}
};

class IntelHex: public Dump {
private:
  unsigned char checksum;
  unsigned int addr;
  void print_byte(unsigned char len);
  void begin_line(unsigned char len, unsigned short a, unsigned char type);
  void end_line();
  void address(int ia);
public:
  IntelHex(Print& Lout):Dump(Lout,32) {};
  void line(const char* start0, const char* start1, int base, int len0, int len1) override;
  void begin() override;
  void end() override;
  void line(const char* start, int base, int len) override;
};

//Converts 4 bytes of binary into 5 printable ASCII characters from 33 (!) to 117 (u)
//This implementation does not use the "z" shortcut so that any binary string of 4n bytes
//will be encoded into 5n characters. 
class Base85: public Dump {
private:
  void print_group(const char* p, int len);
public:
  Base85(Print& Lout):Dump(Lout,64) {};
  Base85(Print& Lout, int LpreferredLen):Dump(Lout,LpreferredLen) {};
  void line(const char* start, int base, int len) override;
};

class Hd: public Dump {
public:
  void line(const char* start0, const char* start1, int base, int len0, int len1) override;
  Hd(Print& Lout):Dump(Lout,16) {};
  Hd(Print& Lout, int LpreferredLen):Dump(Lout,LpreferredLen) {};
  void line(const char* start, int base, int len) override;
};
#endif
