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
    enum Modifier
    {
        STATUS = 0,
        SET_TIME,
        SET_MAX,
        SET_MODE,
        SET_VALUE
    } m_modifier;

    Message(Modifier modifier)
        : m_modifier(modifier)
    {
    }

    virtual uint16_t length() = 0;

    static const uint8_t m_length = GENERIC_REPORT_SIZE;

private:
};

class Status : public Message
{
public:
    Status(long)
        : Message(STATUS)
    {}

    uint16_t length()
    {
        return sizeof(this);
    }

    long     m_time;
    uint16_t m_currValues[Led::TOTALCH];
};

class SetTime : public Message
{
public:
    SetTime()
        : Message(SET_TIME)
    {}

    uint16_t length()
    {
        return sizeof(this);
    }

    long     time;
};

class SetMax : public Message
{
public:
    SetMax()
        : Message(SET_MAX)
    {}

    uint16_t length()
    {
        return sizeof(this);
    }

    uint16_t  maxValues[Led::TOTALCH];
};

class SetMode : public Message
{
public:
    SetMode()
        : Message(SET_MODE)
    {}

    // allowable modes
    enum m_mode
    {
        MANUAL,
        SEMI_AUTOMATIC,
        FULLY_AUTOMATIC,
    };

    uint16_t length()
    {
        return sizeof(this);
    }
};
