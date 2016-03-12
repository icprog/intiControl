//-----------------------------------------------------------------------------
//
//  debug.h
//
//  WFF UDB GenericHID Reference Firmware (4_0)
//  A reference firmware for the WFF GenericHID Communication Library
//
//  Copyright (c) 2011 Simon Inns
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//  Web:    http://www.waitingforfriday.com
//  Email:  simon.inns@gmail.com
//
//-----------------------------------------------------------------------------

#pragma once

#include <inttypes.h>

class Debug
{
public:
    Debug();

    void output    (const char * dbgString);
    void sendBuffer(const char * dbgString);

private:
    static const uint8_t DEBUGBUFFERSIZE = 256;
    // Buffer pointers
    uint16_t m_start;
    uint16_t m_end;
    uint16_t m_level;

    // The following array is the cyclic buffer
    uint8_t debugBuffer[DEBUGBUFFERSIZE];
};
