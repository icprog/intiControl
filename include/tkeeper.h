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

#include <inttypes.h>
#include <datetime.h>

class TKeeper
{
public:
    struct location
    {
        float latitude;
        float longitude;
    };

    TKeeper(location & loc, int tz);

    // configuration
    bool setLocation(location &);
    bool setTimezone(int & );
    bool setDstRules(uint8_t,uint8_t,uint8_t,uint8_t,uint8_t);

    // Political
    void GMT(uint8_t *);
    void DST(uint8_t *);

    // Solar
    bool  SunRise  (uint8_t *);
    bool  SunSet   (uint8_t *);
    float MoonPhase(uint8_t *);
    void  Sidereal (uint8_t *, bool);

private:
    location m_location;
    int m_tz;

    uint8_t dstm1, dstw1, dstm2, dstw2, dstadv;

    void Adjust    (uint8_t *, long);
    bool ComputeSun(uint8_t *, bool);

    char   Signum  (int);
    int    Absolute(int);
    long   Absolute(long);
    float  Absolute(float);

    long DayNumber(uint16_t, uint8_t, uint8_t);
    bool InDst(uint8_t *);
    uint8_t DayOfWeek(uint8_t * when);
    uint8_t LengthOfMonth(uint8_t * when);

    bool IsLeapYear(int yr);

    static const float MOON_PERIOD;
    static const int   SECOND = 0;
    static const int   MINUTE = 1;
    static const int   HOUR   = 2;
    static const int   DAY    = 3;
    static const int   MONTH  = 4;
    static const int   YEAR   = 5;
};
