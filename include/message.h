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

#include <inttypes.h>
#include <AppConfig.h>
#include <led.h>

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

    Message(msgType type = UNKNOWN, uint8_t len = sizeof(msgType) + sizeof(uint16_t))
        :m_type(type), m_len(len)
    {}

    operator msgType () const { return m_type; }
    operator uint16_t() const { return m_len;  }

protected:
    msgType  m_type;
    uint16_t m_len;
};

class msgStatus : public Message
{
public:
    msgStatus()
        :Message(STATUS, sizeof(long) + Led::TOTALCH * sizeof(uint16_t))
    {}

    long      m_time;
    uint16_t  m_currValues[Led::TOTALCH];
};

class msgSetTime : public Message
{
public:
    msgSetTime()
        :Message(SET_TIME, sizeof(long))
    {}

    long      m_time;
};

class msgSetMax : public Message
{
public:
    msgSetMax()
        :Message(SET_MAX, sizeof(long) + Led::TOTALCH * sizeof(uint16_t))
    {}

    long      m_time;
    uint16_t  m_maxValues[Led::TOTALCH];
};

class msgSetMode : public Message
{
public:
    msgSetMode()
        :Message(SET_MODE, Led::TOTALCH * sizeof(uint16_t))
    {}

    // allowable modes
    enum mode
    {
        MANUAL,
        SEMI_AUTOMATIC,
        FULLY_AUTOMATIC,
    };

    mode      m_currMode;
};
