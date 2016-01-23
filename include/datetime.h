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
 * DateTime class derived from Arduino RTC library.
 * https://github.com/jcw/rtclib
 *
 */

#pragma once

#include <inttypes.h>

/**
 * @brief The DateTime class
 */
class DateTime
{
public:
    DateTime (long t = 0);
    DateTime (uint16_t year, uint8_t month, uint8_t day,
            uint8_t hour = 0, uint8_t min = 0, uint8_t sec = 0);
    DateTime (const char* date, const char* time);

    const uint16_t year     () const { return 2000 + yOff;  }
    const uint8_t  shortyear() const { return yOff;}
    const uint8_t  month    () const { return m;   }
    const uint8_t  day      () const { return d;   }
    const uint8_t  hour     () const { return hh;  }
    const uint8_t  minute   () const { return mm;  }
    const uint8_t  second   () const { return ss;  }
    const uint8_t  dayOfWeek() const;

    // 32-bit times as seconds since 1/1/2000
    long get() const;

private:
    uint8_t yOff, m, d, hh, mm, ss;

    static const long SECONDS_PER_DAY = 86400L;
};
