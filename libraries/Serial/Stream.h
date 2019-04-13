/*
  Stream.h - base class for character-based streams.
  Copyright (c) 2010 David A. Mellis.  All right reserved.

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

/*This is a purely abstract class, so an Interface in Java terms. We use this
  primarily to add input support to the Print class, which provides all the 
  output support.  */

#ifndef Stream_h
#define Stream_h

#include <inttypes.h>
#include "Print.h"

class Stream : public Print {
  public:
    virtual int available() = 0; ///<Gets the number of bytes available in the stream. This is only for bytes that have already arrived. 
    virtual int read() = 0;      ///<Read a byte from the stream and advance to the next one
    virtual int peek() = 0;      ///<Read a byte from the file without advancing to the next one. That is, successive calls to peek() will return the same value, as will the next call to read(). 
    virtual void flush() = 0;    ///<Flush the input buffer - equivalent to calling read() until available()==0, and discarding the bytes.
};

#endif
