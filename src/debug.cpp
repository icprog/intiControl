//-----------------------------------------------------------------------------
//
//  debug.c
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

// Global includes
#include <avr/io.h>
#include <string.h>

// Local includes
#include "debug.h"
#include "descriptors.h"

Debug::Debug()
    : m_start(0), m_end(0), m_level(0)
{
}

// Send debug text to the debug log
void Debug::output(const char* dbgString)
{
	uint8_t charNumber;
	
	// Is there space in the debug buffer?
    if (m_level + strlen((const char *)dbgString) >= DEBUGBUFFERSIZE - 2)
	{
		// Buffer does not have enough space... silently drop the debug string
	}
	else
	{	
		// Buffer is not full, write the bytes and update the end pointer
        for (charNumber = 0; charNumber < strlen((const char *)dbgString); charNumber++)
		{
            debugBuffer[m_end] = dbgString[charNumber];
            m_end = (m_end + 1) % DEBUGBUFFERSIZE;
			
			// Increment the buffer level indicator
            m_level++;
		}
		
		// Add a return and new line to the end of the string
        debugBuffer[m_end] = '\r';
        m_end = (m_end + 1) % DEBUGBUFFERSIZE;
        m_level++;

        debugBuffer[m_end] = '\n';
        m_end = (m_end + 1) % DEBUGBUFFERSIZE;
        m_level++;
    }
}

// Copy 63 bytes of the debug buffer to the USB send buffer
// The first byte is the number of characters transferred
void Debug::sendBuffer(char *sndBuffer)
{
    // Determine the number of bytes to send
    uint16_t bytesToSend = (m_level > GENERIC_REPORT_SIZE - 1) ? (GENERIC_REPORT_SIZE - 1) : m_level;

	// Place the number of sent bytes in byte[0] of the send buffer
    sndBuffer[0] = bytesToSend - 1;
	
    if (m_level != 0)
	{
        for (uint16_t byteCounter = 1; byteCounter < bytesToSend; byteCounter++)
		{
			// Send the next byte to the send buffer
            sndBuffer[byteCounter] = debugBuffer[m_start];
			
			// Update the cyclic buffer pointer
            m_start = (m_start + 1) % DEBUGBUFFERSIZE;
			
			// Decrement the buffer level indicator
            m_level--;
		}
	}
    else
        sndBuffer[0] = 0;
}
