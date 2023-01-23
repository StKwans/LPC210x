#ifndef dump_h
#define dump_h

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
  void region(const char* start, int base, int len, int rec_len) {
    begin();
    const char* p=start;
    while(len>0) {
      if(rec_len>len) rec_len=len;
      line(p,base,rec_len);
      base+=rec_len;
      p+=rec_len;
      len-=rec_len;
    }
    end();
  }
  void region(const char* start, int len, int rec_len) {
    region(start,(int)start,len,rec_len);
  }
  void region(const char* start, int len) {
    region(start,0,len,preferredLen);
  }
  //Display two regions as if they were contiguous. Useful for going around
  //the corner of a circular buffer
  void region(const char* start0, const char* start1, int base, int len0, int len1, int rec_len) {
    begin();
    const char* p0=start0;
    const char* p1=start1;
    //Print the first part complete lines
    while(len0>=rec_len) {
      line(p0,base,rec_len);
      base+=rec_len;
      p0+=rec_len;
      len0-=rec_len;
    }
    int len_mid=(rec_len-len0)>len1?len1:(rec_len-len0);
    line(p0,p1,base,len0,len_mid);
    p1+=len_mid;
    len1-=len_mid;
    base+=len_mid+len0;
    while(len1>0) {
      if(rec_len>len1) rec_len=len1;
      line(p1,base,rec_len);
      base+=rec_len;
      p1+=rec_len;
      len1-=rec_len;
    }
    end();
  }
  void region(const char* start0, const char* start1, int base, int len0, int len1) {
    region(start0,start1,base,len0,len1,preferredLen);
  }
  //The following matches the above for printing a default base, but "default base"
  //doesn't make sense for two consecutive regions.
  //void region(const char* start0, const char* start1, int len0, int len1) {region(start0,start1,(int)start0,len0,len1,preferredLen);};
  void dumpText() {
    region(btext,etext-btext,preferredLen);
  }
  void dumpSource() {
    region(source_start,0,source_end-source_start,preferredLen);
  }
};

class IntelHex: public Dump {
private:
  unsigned char checksum;
  unsigned int addr;
  void print_byte(unsigned char b) {
    checksum+=b;
    out.print(b>>4,HEX);
    out.print(b & 0x0F,HEX);
  }
  void begin_line(unsigned char len, unsigned short a, unsigned char type) {
    checksum=0;
    out.print(":");
    print_byte(len);
    print_byte(a>>8);
    print_byte(a & 0xFF);
    print_byte(type);
  }
  void end_line() {
    print_byte(256-checksum);
    out.println();
  }
  void address(int ia){
    if((ia & 0xFFFF0000) != (addr & 0xFFFF0000)) {
      addr=ia;
      begin_line(2,0,4);
      print_byte((addr>>24) & 0xFF);
      print_byte((addr>>16) & 0xFF);
      end_line();
    }
  }
public:
  IntelHex(Print& Lout):Dump(Lout,32) {};
  void line(const char* start0, const char* start1, int base, int len0, int len1) override {
    address(base);
    begin_line(len0+len1,((unsigned int)base)&0xFFFF,0);
    for(int i=0;i<len0;i++) print_byte(start0[i]);
    for(int i=0;i<len1;i++) print_byte(start1[i]);
    end_line();
  }
  void begin() override {
    addr=0;
  }
  void end() override{
    begin_line(0,0,1);
    end_line();
  }
  void line(const char* start, int base, int len) override {
    address(base);
    begin_line(len,((unsigned int)base)&0xFFFF,0);
    for(int i=0;i<len;i++) print_byte(start[i]);
    end_line();
  }
};

//Converts 4 bytes of binary into 5 printable ASCII characters from 33 (!) to 117 (u)
//This implementation does not use the "z" shortcut so that any binary string of 4n bytes
//will be encoded into 5n characters. 
class Base85: public Dump {
private:
  void print_group(const char* p, int len) {
    unsigned int group=0;
    for(int i=0;i<4;i++) {
      group<<=8;
      if(i<len) group |= p[i];
    }
    char group_c[5];
    for(int i=0;i<5;i++) {
      group_c[4-i]=(group % 85)+33;
      group/=85;
    }
    for(int i=0;i<len+1;i++) out.print(group_c[i]);
  }
public:
  Base85(Print& Lout):Dump(Lout,64) {}
  Base85(Print& Lout, int LpreferredLen):Dump(Lout,LpreferredLen) {}
  void line(const char* start, int base, int len) override {
    while(len>0) {
      print_group(start,len>4?4:len);
      start+=4;
      len-=4;
    }
    out.println();
  }
};

class Hd: public Dump {
public:
  void line(const char* start0, const char* start1, int base, int len0, int len1) override {
    out.print(base,HEX,4);
    out.print(' ');
    for(int i=0;i<len0;i++) {
      out.print(start0[i],HEX,2);
      if(i%4==3) out.print(' ');
    }
    for(int i=0;i<len1;i++) {
      out.print(start1[i],HEX,2);
      if((i+len0)%4==3) out.print(' ');
    }
    for(int i=len0+len1;i<preferredLen;i++) {
      out.print("  ");
      if(i%4==3) out.print(' ');
    }
    out.print(' ');
    for(int i=0;i<len0;i++) out.print((start0[i]>=32 && start0[i]<127)?start0[i]:'.');
    for(int i=0;i<len1;i++) out.print((start1[i]>=32 && start1[i]<127)?start1[i]:'.');
    out.println();
  }
  Hd(Print& Lout):Dump(Lout,16) {}
  Hd(Print& Lout, int LpreferredLen):Dump(Lout,LpreferredLen) {}
  void line(const char* start, int base, int len) override {
    out.print(base,HEX,4);
    out.print(' ');
    for(int i=0;i<len;i++) {
      out.print(start[i],HEX,2);
      if(i%4==3) out.print(' ');
    }
    for(int i=len;i<preferredLen;i++) {
      out.print("  ");
      if(i%4==3) out.print(' ');
    }
    out.print(' ');
    for(int i=0;i<len;i++) out.print((start[i]>=32 && start[i]<127)?start[i]:'.');
    out.println();
  }
};

#endif
