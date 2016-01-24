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

#include <math.h>
#include <stdlib.h>

#include "tkeeper.h"

const float TKeeper::MOON_PERIOD = 29.530588853f;

TKeeper::TKeeper(const location & loc, int tz)
    : m_location(loc), m_tz(tz)
{
    setDstRules(3, 2, 11, 1, 60);
}

bool TKeeper::setTimezone(int & tz)
{
    bool ret = (abs(tz) < 720);

    if (ret)
        m_tz = tz;

    return ret;
}

bool TKeeper::setLocation(const location & loc)
{
    bool ret = (fabs(loc.longitude) < 180.0) && (fabs(loc.latitude) < 90.0);

    if (ret)
        m_location = loc;

    return ret;
}

bool TKeeper::setDstRules(uint8_t sm, uint8_t sw, uint8_t em, uint8_t ew, uint8_t adv)
{
    if (sm == 0 || sw == 0 || em == 0 || ew == 0)
        return false;

    if (sm > 12 || sw > 4 || em > 12 || ew > 4)
        return false;

    dstm1  = sm;
    dstw1  = sw;
    dstm2  = em;
    dstw2  = ew;
    dstadv = adv;

    return true;
}

void TKeeper::GMT(DateTime &now)
{
    now.adjust(-m_tz);
}

void TKeeper::DST(DateTime &now)
{
    if (now.InDst(now))
        now.adjust(dstadv);
}

bool TKeeper::SunRise(DateTime &when)
{
    return ComputeSun(when, true);
}

bool TKeeper::SunSet(DateTime &when)
{
    return ComputeSun(when, false);
}

float TKeeper::MoonPhase(const DateTime & when)
{
    // we compute the number of days since Jan 6, 2000
    // at which time the moon was 'new'
    long  d = DayNumber(2000 + when.year(), when.month(), when.day()) - DayNumber(2000, 1, 6);
    float p = d / MOON_PERIOD;

    d  = p;
    p -= d; // p is now the fractional cycle, 0 to (less than) 1

    return p;
}

void TKeeper::Sidereal(DateTime &when, bool local)
{
    uint64_t second, d;
    long minute;

/*
   Based on US Naval observatory GMST algorithm
   (http://aa.usno.navy.mil/faq/docs/GAST.php)
   Adapted for Arduino
   -----------------------------------------------

   Since Arduino doesn't provide double precision floating point, we have
   modified the algorithm to use (mostly) integer math.

   This implementation will work until the year 2100 with residual error +- 2 seconds.

   That translates to +-30 arc-seconds of angular error, which is just
   about the field of view of a 100x telescope, and well within the field of
   view of a 50x scope.
*/

    // we're working in GMT time
    GMT(when);

    // Get number of days since our epoch of Jan 1, 2000
    d = DayNumber(when.year() + 2000, when.month(), when.day()) - DayNumber(2000, 1, 1);

    // compute calendar seconds since the epoch
    second = d * 86400LL + when.hour() * 3600LL + when.minute() * 60LL + when.second();

    // multiply by ratio of calendar to sidereal time
    second *= 1002737909LL;
    second /= 1000000000LL;

    // add sidereal time at the epoch
    second += 23992LL;

    // convert from gmt to local
    if (local)
    {
        d = 240.0 * m_location.longitude;
        second += d;
    }

    // constrain to 1 calendar day
    second %= 86400LL;

    // update the tl_time array
    minute = second / 60LL;
    d = minute * 60LL;

    when.resetTime(0, 0, second - d);

    when.adjust(minute);
}



//====Utility====================

// rather than import yet another library, we define sgn and abs ourselves
long TKeeper::Absolute(long n)
{
    if (n < 0)
        return 0 - n;

    return n;
}

float TKeeper::Absolute(float n)
{
    if (n < 0)
        return 0 - n;

    return n;
}
int TKeeper::Absolute(int n)
{
    if (n < 0)
        return 0 - n;

    return n;
}
/*
void TKeeper::Adjust(DateTime *when, long offset)
{
    long tmp, mod, nxt;

    // offset is in minutes
    tmp = when->minute() + offset; // minutes
    nxt = tmp / 60;// hours
    mod = Absolute(tmp) % 60;
    mod = mod * Signum(tmp) + 60;
    mod %= 60;
    when[MINUTE] = mod;

    tmp = nxt + when->hour();
    nxt = tmp / 24;// days
    mod = Absolute(tmp) % 24;
    mod = mod*Signum(tmp) + 24;
    mod %= 24;
    when[HOUR]=mod;

    tmp = nxt + when->day();
    mod = LengthOfMonth(when);

    if (tmp > mod)
    {
        tmp -= mod;
        when[DAY] = tmp + 1;
        when[MONTH]++;
    }
    if (tmp < 1)
    {
        when[MONTH]--;
        mod = LengthOfMonth(when);
        when[DAY] = tmp + mod;
    }

    tmp=when[YEAR];
    if(when[MONTH] == 0)
    {
        when[MONTH] = 12;
        tmp--;
    }

    if(when[MONTH] > 12)
    {
        when[MONTH] = 1;
        tmp++;
    }

    tmp += 100;
    tmp %= 100;
    when[YEAR] = tmp;
}
*/
bool TKeeper::ComputeSun(DateTime &when, bool rs)
{
    uint8_t month = when.month() - 1;
    uint8_t day   = when.day()   - 1;

    float lon = -m_location.longitude / 57.295779513082322;
    float lat =  m_location.latitude  / 57.295779513082322;

    //approximate hour;
    uint8_t a = (rs) ? 18 : 6;

    // approximate day of year
    // between 0 and 365
    float y = month * 30.4375 + day  + a / 24.0;

    // compute fractional year
    // 0... 1
    y *= 1.718771839885e-02;

    // compute equation of time... .43068174
    float eqt  = 229.18 * (0.000075 + 0.001868 * cos(y) - 0.032077 * sin(y) - 0.014615 * cos(y * 2) - 0.040849 * sin(y * 2));

    // compute solar declination... -0.398272
    float decl = 0.006918  -0.399912 * cos(y) + 0.070257 * sin(y) - 0.006758 * cos(y * 2) + 0.000907 * sin(y * 2) - 0.002697 * cos(y * 3) + 0.00148 * sin(y * 3);

    // compute hour angle
    float ha = (cos(1.585340737228125) / (cos(lat) * cos(decl)) - tan(lat) * tan(decl));

    // we're in the (ant)arctic and there is no rise(or set) today!
    if (fabs(ha) > 1.0)
        return false;

    ha = acos(ha);
    if (rs == false)
        ha = -ha;

    // compute minutes from midnight
    int minutes = 720 + 4 * (lon - ha) * 57.295779513082322 - eqt;

    // convert from UTC back to our timezone
    minutes += m_tz;

    // adjust the time array by minutes
    when.resetTime();

    when.adjust(minutes);

    return true;
}

long TKeeper::DayNumber(uint16_t y, uint8_t m, uint8_t d)
{

    m = (m + 9) % 12;
    y = y - m / 10;

    return 365 * y + y / 4 - y / 100 + y / 400 + (m * 306 + 5) / 10 + d - 1;
}
bool TKeeper::InDst(const DateTime & p)
{
    // input is assumed to be standard time
    char nSundays, prevSunday, weekday;

    if(p.month()< dstm1 || p.month() > dstm2)
        return false;

    if(p.month() > dstm1 && p.month() < dstm2)
        return true;

    // if we get here, we are in either the start or end month

    // How many sundays so far this month?
    weekday    = p.dayOfWeek();
    nSundays   = 0;
    prevSunday = p.day() - weekday + 1;

    if(prevSunday > 0)
    {
        nSundays = prevSunday / 7;
        nSundays++;
    }

    if(p.month() == dstm1)
    {
        if(nSundays < dstw1) return false;
        if(nSundays > dstw1) return true;
        if(weekday > 1) return true;
        if(p.hour() > 1) return true;

        return false;
    }

    if(nSundays < dstw2) return true;
    if(nSundays > dstw2) return false;
    if(weekday > 1) return false;
    if(p.hour() > 1) return false;

    return true;
}
