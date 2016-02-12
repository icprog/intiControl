/*
 * This file is part of intiLED.
 *
 * intiLED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * intiLED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with intiLED.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#pragma once

#ifndef HOST_CTRL
#include <inttypes.h>
#include <descriptors.h>
#include <led.h>

#define NUMBER_OF_CHANNELS Led::TOTALCH
#else
/** Size in bytes of the Generic HID reporting endpoint. */
#define GENERIC_EPSIZE            24
#define NUMBER_OF_CHANNELS         7
#endif


// To make things easier on the device, we assume that all messages are of
// the same length, defined by GENERIC_EPSIZE
// This allows us to store an array of received messages that can be processed
// in "slow" time by the controller
//
// To accomplish this, we simply allocate enough memory in the array to hold
// a predefined number of messages.
// Then each memory location is read by casting to Message
// We can then read the msgType and perform one more cast to get the actual message


class Message
{
public:
    enum msgType
    {
        UNKNOWN,
        STATUS,
        SET_TIME,
        SET_MAX,
        SET_MODE
    };

    Message(msgType type = UNKNOWN)
        :m_type(type), m_len(GENERIC_EPSIZE)
    {}

    operator msgType () const { return m_type; }
    operator uint16_t() const { return m_len;  }
    operator uint8_t () const { return m_len;  }

protected:
    msgType  m_type;
    const uint8_t m_len;
};

#define HEADER_LENGTH 2


class msgStatus : public Message
{
public:
    msgStatus()
        :Message(STATUS)
    {}

    uint32_t  m_time;
    uint16_t  m_currValues[NUMBER_OF_CHANNELS];

    // actual message size is
    // header + 4 + 2 * 7
    uint8_t   m_pad[GENERIC_EPSIZE - (HEADER_LENGTH + 4 + 2 * 7)];
};

class msgSetTime : public Message
{
public:
    msgSetTime()
        :Message(SET_TIME)
    {}

    uint32_t  m_time;

    // actual message size is
    // header + 4
    uint8_t   m_pad[GENERIC_EPSIZE - (HEADER_LENGTH + 4)];
};

class msgSetMax : public Message
{
public:
    msgSetMax()
        :Message(SET_MAX)
    {}

    uint32_t  m_time;
    uint16_t  m_maxValues[NUMBER_OF_CHANNELS];

    // actual message size is
    // header + 4 + 2 * 7
    uint8_t   m_pad[GENERIC_EPSIZE - (HEADER_LENGTH + 4 + 2 * 7)];
};

class msgSetMode : public Message
{
public:
    msgSetMode()
        :Message(SET_MODE)
    {}

    // allowable modes
    enum mode
    {
        MANUAL,
        SEMI_AUTOMATIC,
        FULLY_AUTOMATIC,
    };

    mode m_currMode;

    // actual message size is
    // header + 1
    uint8_t   m_pad[GENERIC_EPSIZE - (HEADER_LENGTH + 1)];
};
