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
 * TKeeper class derived from TimeLord library.
 * http://swfltek.com/arduino/sunrise.html
 *
 * The above library is deprecated and functionality is listed as being
 * incorporated into avr-libc 1.8.1
 *
 * Should look at using standard library functions instead
 *
 */

#include <inttypes.h>
#include <datetime.h>

class TKeeper
{
public:
    class location
    {
    public:
        // default location for barrier reef
        location(float lat = 18.2861, float lon = 147.7000)
            : latitude(lat), longitude(lon)
        {}

        float latitude;
        float longitude;
    };

    // default timezone for Sydney, Australia
    TKeeper(const location & loc, int tz = -600);

    // configuration
    bool setLocation(const location &);
    bool setTimezone(int & );
    bool setDstRules(uint8_t,uint8_t,uint8_t,uint8_t,uint8_t);

    // Political
    void GMT(DateTime & now);
    void DST(DateTime & now);

    // Solar
    bool  SunRise  (DateTime &);
    bool  SunSet   (DateTime &);
    float MoonPhase(const DateTime &);
    void  Sidereal (DateTime &, bool);

    bool InDst(const DateTime & p);

private:
    location m_location;
    // number of minutes to offset for timezone
    int m_tz;

    uint8_t dstm1, dstw1, dstm2, dstw2, dstadv;

    //void Adjust    (DateTime &, long);
    bool ComputeSun(DateTime &, bool);

    int    Absolute(int);
    long   Absolute(long);
    float  Absolute(float);

    long DayNumber(uint16_t, uint8_t, uint8_t);
    //bool InDst(uint8_t *);
    //uint8_t DayOfWeek(uint8_t * when);
    //uint8_t LengthOfMonth(uint8_t * when);

    bool IsLeapYear(int yr);

    static const float MOON_PERIOD;
    static const int   SECOND = 0;
    static const int   MINUTE = 1;
    static const int   HOUR   = 2;
    static const int   DAY    = 3;
    static const int   MONTH  = 4;
    static const int   YEAR   = 5;
};
