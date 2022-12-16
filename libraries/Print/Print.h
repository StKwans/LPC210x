/*
  Print.h - Base class that provides print() and println()
  Copyright (c) 2008 David A. Mellis.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef Print_h
#define Print_h

//#include "WString.h"

//#define U64

const int DEC=10;
const int HEX=16;
const int OCT=8;
const int BIN=2;
const int BYTE=0;

#include <stdarg.h>

class Print {
private:
  void printNumber(unsigned int n, int base, int digits) {
    unsigned char buf[8 * sizeof(unsigned int)];
    unsigned int i = 0;

    if (n == 0) {
      for(i=0;i<(digits>0?digits:1);i++) print('0');
      return;
    }

    while (n > 0||digits>0) {
      buf[i] = n % base;
      i++;
      digits--;
      n /= base;
    }

    for (; i > 0; i--) print((char) (buf[i - 1] < 10 ?'0' + buf[i - 1]:'A' + buf[i - 1] - 10));
  }
  void printNumber(unsigned long long n, int base, int digits) {
    unsigned char buf[8 * sizeof(unsigned long long)];
    unsigned int i = 0;

    if (n == 0) {
      for(i=0;i<(digits>0?digits:1);i++) print('0');
      return;
    }

    while (n > 0||digits>0) {
      buf[i] = n % base;
      i++;
      digits--;
      n /= base;
    }

    for (; i > 0; i--) print((char) (buf[i - 1] < 10 ?'0' + buf[i - 1]:'A' + buf[i - 1] - 10));
  }
  void printFloat(float number, unsigned char digits) {
    // Handle negative numbers
    if (number < 0.0) {
      print('-');
      number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"
    fp rounding = 0.5;
    for (unsigned char i=0; i<digits; ++i) {
      rounding /= 10.0;
    }

    number += rounding;

    // Extract the integer part of the number and print it
    unsigned int int_part = (unsigned int)number;
    fp remainder = number - (fp)int_part;
    print(int_part);

    // Print the decimal point, but only if there are digits beyond
    if (digits > 0) {
      print(".");
    }

    // Extract digits from the remainder one at a time
    while (digits-- > 0) {
      remainder *= 10.0;
      unsigned int toPrint = (unsigned int)(remainder);
      print(toPrint);
      remainder -= toPrint;
    }
  }
  // Constants used by printf
  static const int SCRATCH=12;  //32Bits go up to 4GB + 1 Byte for \0

public:
  virtual void write(unsigned char) {};
  virtual void write(const char *str) {
    while (*str) write(*str++);
  }
  virtual void write(const char *buffer, size_t size) {
    while (size--) write(*buffer++);
  }

  void print(const char *str) {
    write(str);
  };
  void print(char c, int base=BYTE, int digits=0){
    print((int) c, base,digits);
  };
  void print(unsigned char b, int base=BYTE,int digits=0){
    print((unsigned int) b, base,digits);
  };
  void print(int n, int base=DEC, int digits=0) {
    if (base == 0) {
      write(n);
    } else if (base == 10) {
      if (n < 0) {
        print('-');
        n = -n;
      }
      printNumber((unsigned int)n, 10, digits);
    } else {
      printNumber((unsigned int)n, base,digits);
    }
  }
  void print(unsigned int n, int base=DEC, int digits=0) {
    if (base == 0) write(n);
    else printNumber(n, base, digits);
  }

  void print(long long n, int base=DEC, int digits=0) {
    if (base == 0) {
      write(n);
    } else if (base == 10) {
      if (n < 0) {
        print('-');
        n = -n;
      }
      printNumber((unsigned long long)n, 10, digits);
    } else {
      printNumber((unsigned long long)n, base, digits);
    }
  }
  void print(unsigned long long int n, int base= DEC,int digits=0) {
    if (base == 0) write(n);
    else printNumber(n, base);
  };
  void print(fp n, int digits=2) {
    printFloat(n, digits);
  };

  void println(const char *c){
    print(c);
    println();
  };
  void println(char c, int base=BYTE, int digits=0){
    print((char)c, base,digits);
    println();
  }
  void println(unsigned char b, int base=BYTE, int digits=0){
    print((unsigned char)b, base,digits);
    println();
  }
  void println(int n, int base=DEC, int digits=0) {
    print(n, base,digits);
    println();
  }
  void println(unsigned int n, int base=DEC, int digits=0) {
    print(n, base, digits);
    println();
  }
  void println(long long int n, int base=DEC, int digits=0) {
    print(n, base,digits);
    println();
  }
  void println(unsigned long long n, int base=DEC,int digits=0) {
    print(n, base,digits);
    println();
  }
  void println(float n, int digits) {
    print(n, digits);
    println();
  }
  void println(void) {
    print('\r');print('\n');
  };
  void printf(char const *format, ...) {
    //#########################################################################
    // printf.c
    //
    // *** printf() based on sprintf() from gcctest9.c Volker Oth
    //
    // *** Changes made by Holger Klabunde
    // Now takes format strings from FLASH (was into RAM ! before)
    // Fixed bug for %i, %I. u_val was used before it had the right value
    // Added %d, %D (is same as %i, %I)
    // Support for long variables %li, %ld, %Lu, %LX ....
    // %x, %X now gives upper case hex characters A,B,C,D,E,F
    // Output can be redirected in a single function: myputchar()
    // Make printf() smaller by commenting out a few #defines
    // Added some SPACE and ZERO padding %02x or % 3u up to 9 characters
    //
    // Todo:
    // %f, %F for floating point numbers
    //
    // *** Changes made by Martin Thomas for the efsl debug output:
    // - removed AVR "progmem"
    // - added function pointer for "putchar"
    // - devopen function
    //
    //#########################################################################
    unsigned char scratch[SCRATCH];
    unsigned char format_flag;
    unsigned short base;
    unsigned char *ptr;
    unsigned char issigned=0;
    va_list ap;

    unsigned int u_val=0;
    int s_val=0;

    unsigned char fill;
    unsigned char width;

    va_start(ap, format);
    for (;;) {
      while ((format_flag = *(format++)) != '%') {
        // Until '%' or '\0'
        if (!format_flag) {
          va_end(ap);
          return;
        }
        write(format_flag);
      }

      issigned=0; //default unsigned
      base = 10;

      format_flag = *format++; //get char after '%'

      width=0; //no formatting
      fill=0;  //no formatting
      if(format_flag=='0' || format_flag==' ') {
        //SPACE or ZERO padding  ?
        fill=format_flag;
        format_flag = *format++; //get char after padding char
        if(format_flag>='0' && format_flag<='9') {
          width=format_flag-'0';
          format_flag = *format++; //get char after width char
        }
      }

      switch (format_flag) {
        case 'c':
        case 'C':
          format_flag = va_arg(ap,int);
          // no break -> run into default
        default:
          write(format_flag);
          continue;
        case 'S':
        case 's':
          ptr = (unsigned char*)va_arg(ap,char *);
          while(*ptr) {
            write(*ptr);
            ptr++;
          }
          continue;
        case 'o':
        case 'O':
          base = 8;
          write('0');
          goto CONVERSION_LOOP;
        case 'i':
        case 'I':
        case 'd':
        case 'D':
          issigned=1;
          // no break -> run into next case
        case 'u':
        case 'U':
          //don't insert some case below this if USE_HEX is undefined !
          //or put       goto CONVERSION_LOOP;  before next case.
          goto CONVERSION_LOOP;
        case 'x':
        case 'X':
          base = 16;
        CONVERSION_LOOP:
          if(issigned) {
            //Signed types
            s_val = va_arg(ap,int);

            if(s_val < 0) {
              //Value negative ?
              s_val = - s_val; //Make it positive
              write('-');    //Output sign
            }

            u_val = (unsigned int)s_val;
          } else {
            //Unsigned types
            u_val = va_arg(ap,unsigned int);
          }

          ptr = scratch + SCRATCH;
          *--ptr = 0;
          do {
            char ch = u_val % base + '0';
            if (ch > '9') {
              ch += 'a' - '9' - 1;
              ch-=0x20;
            }
            *--ptr = ch;
            u_val /= base;

            if(width) width--; //calculate number of padding chars
          } while (u_val);

          while(width--) {
            *--ptr = fill; //insert padding chars
          }

          while(*ptr) {
            write(*ptr);
            ptr++;
          }
      }
    }
  }
};

#endif




