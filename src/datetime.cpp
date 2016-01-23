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

#include <avr/pgmspace.h>
#include <datetime.h>

static const uint8_t daysInMonth [] PROGMEM = {
  31,28,31,30,31,30,31,31,30,31,30,31
};

// number of days since 2000/01/01, valid for 2001..2099
uint16_t date2days(uint16_t y, uint8_t m, uint8_t d)
{
    if (y >= 2000)
        y -= 2000;
    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i)
        days += pgm_read_byte(daysInMonth + i - 1);
    if (m > 2 && y % 4 == 0)
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}

long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s)
{
    return ((days * 24L + h) * 60 + m) * 60 + s;
}

uint8_t conv2d(const char* p)
{
    uint8_t v = 0;

    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}

DateTime::DateTime (long t)
    : yOff(0), m(0), d(0), hh(0), mm(0), ss(0)
{
    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;

    for (yOff = 0; ; ++yOff)
    {
        leap = yOff % 4 == 0;
        if (days < (uint16_t)365 + leap)
            break;
        days -= 365 + leap;
    }

    for (m = 1; ; ++m)
    {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }

    d = days + 1;
}

DateTime::DateTime (uint16_t year,
                    uint8_t  month,
                    uint8_t  day,
                    uint8_t  hour,
                    uint8_t  min,
                    uint8_t  sec)
    : yOff((year >= 2000) ? (year - 2000) : year),
      m(month), d(day), hh(hour), mm(min), ss(sec)
{
}

DateTime::DateTime (const char* date, const char* time)
    : yOff(conv2d(date + 9)), m(0), d(conv2d(date + 4)), hh(conv2d(time)), mm(conv2d(time + 3)), ss(conv2d(time + 6))
{
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch (date[0]) {
        case 'J': m = (date[1] == 'a') ? 1 : (date[2] == 'n') ? 6 : 7; break;
        case 'F': m = 2; break;
        case 'A': m = date[2] == 'r' ? 4 : 8; break;
        case 'M': m = date[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
    }
}

const uint8_t DateTime::dayOfWeek() const
{
    // Jan 1, 2000 is a Saturday, i.e. returns 6
    uint16_t day = get() / SECONDS_PER_DAY;
    return (day + 6) % 7;
}

long DateTime::get() const
{
    uint16_t days = date2days(yOff, m, d);
    return time2long(days, hh, mm, ss);
}
