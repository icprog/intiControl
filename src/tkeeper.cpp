
#include <math.h>
#include <stdlib.h>

#include "tkeeper.h"

const float TKeeper::MOON_PERIOD = 29.530588853f;

TKeeper::TKeeper(TKeeper::location & loc, int tz)
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

bool TKeeper::setLocation(location & loc)
{
    bool ret = (Absolute(loc.longitude) < 180.0) && (Absolute(loc.latitude) < 90.0);

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

void TKeeper::GMT(uint8_t * now)
{
    Adjust(now, -m_tz);
}

void TKeeper::DST(uint8_t *now)
{
    if (InDst(now))
        Adjust(now, dstadv);
}

bool TKeeper::SunRise(uint8_t * when)
{
    return ComputeSun(when, true);
}

bool TKeeper::SunSet(uint8_t * when)
{
    return ComputeSun(when, false);
}

float TKeeper::MoonPhase(uint8_t * when)
{
    // we compute the number of days since Jan 6, 2000
    // at which time the moon was 'new'
    long  d = DayNumber(2000 + when[YEAR], when[MONTH], when[DAY]) - DayNumber(2000, 1, 6);
    float p = d / MOON_PERIOD;

    d  = p;
    p -= d; // p is now the fractional cycle, 0 to (less than) 1

    return p;
}

void TKeeper::Sidereal(uint8_t * when, bool local)
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
    d = DayNumber(when[YEAR] + 2000, when[MONTH], when[DAY]) - DayNumber(2000, 1, 1);

    // compute calendar seconds since the epoch
    second = d * 86400LL + when[HOUR] * 3600LL + when[MINUTE] * 60LL + when[SECOND];

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

    when[SECOND] = second - d;
    when[HOUR]   = 0;
    when[MINUTE] = 0;

    Adjust(when, minute);
}

uint8_t TKeeper::DayOfWeek(uint8_t * when)
{
    int      year  = when[YEAR] + 2000;
    uint8_t  month = when[MONTH];
    uint8_t  day   = when[DAY];

    if (month < 3)
    {
        month += 12;
        year--;
    }

    day = ((13 * month + 3) / 5 + day + year + year / 4 - year / 100 + year / 400 ) % 7;
    day = (day + 1) % 7;

    return day + 1;
}

uint8_t TKeeper::LengthOfMonth(uint8_t * when)
{
    int     yr   = when[YEAR] + 2000;
    uint8_t mnth = when[MONTH];

    if (mnth == 2)
    {
        if (IsLeapYear(yr))
            return 29;
        return 28;
    }

    uint8_t odd = (mnth & 1) == 1;

    if (mnth > 7)
        odd  = !odd;

    if (odd)
        return 31;

    return 30;
}

bool TKeeper::IsLeapYear(int yr)
{
    return ( (yr % 4 == 0 && yr % 100 != 0) || yr % 400 == 0);
}

bool TKeeper::InDst(uint8_t * p)
{
    // input is assumed to be standard time
    char nSundays, prevSunday, weekday;

    if(p[MONTH]<dstm1 || p[MONTH]>dstm2) return false;
    if(p[MONTH]>dstm1 && p[MONTH]<dstm2) return true;

    // if we get here, we are in either the start or end month

    // How many sundays so far this month?
    weekday=DayOfWeek(p);
    nSundays=0;
    prevSunday=p[DAY]-weekday+1;
    if(prevSunday>0){
        nSundays=prevSunday/7;
        nSundays++;
    }

    if(p[MONTH]==dstm1){
        if(nSundays<dstw1) return false;
        if(nSundays>dstw1) return true;
        if(weekday>1) return true;
        if(p[HOUR]>1) return true;
        return false;
    }

    if(nSundays<dstw2) return true;
    if(nSundays>dstw2) return false;
    if(weekday>1) return false;
    if(p[HOUR]>1) return false;
    return true;
}


//====Utility====================

// rather than import yet another library, we define sgn and abs ourselves
char TKeeper::Signum(int n)
{
    if (n < 0)
        return -1;

    return 1;
}
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

void TKeeper::Adjust(uint8_t * when, long offset)
{
    long tmp, mod, nxt;

    // offset is in minutes
    tmp=when[MINUTE]+offset; // minutes
    nxt=tmp/60;// hours
    mod=Absolute(tmp) % 60;
    mod=mod*Signum(tmp)+60;
    mod %= 60;
    when[MINUTE]=mod;

    tmp=nxt+when[HOUR];
    nxt=tmp/24;// days
    mod=Absolute(tmp) % 24;
    mod=mod*Signum(tmp)+24;
    mod %= 24;
    when[HOUR]=mod;

    tmp=nxt+when[DAY];
    mod=LengthOfMonth(when);

    if(tmp>mod){
        tmp-=mod;
        when[DAY]=tmp+1;
        when[MONTH]++;
    }
    if(tmp<1){
        when[MONTH]--;
        mod=LengthOfMonth(when);
        when[DAY]=tmp+mod;
    }

    tmp=when[YEAR];
    if(when[MONTH]==0){
        when[MONTH]=12;
        tmp--;
    }
    if(when[MONTH]>12){
        when[MONTH]=1;
        tmp++;
    }
    tmp+=100;
    tmp %= 100;
    when[YEAR]=tmp;

}

bool TKeeper::ComputeSun(uint8_t * when, bool rs)
{
    float z;
    int doy;

    uint8_t month = when[MONTH] - 1;
    uint8_t day   = when[DAY] - 1;

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
    if (Absolute(ha) > 1.0)
        return false;

    ha = acos(ha);
    if (rs == false)
        ha = -ha;

    // compute minutes from midnight
    int minutes = 720 + 4 * (lon - ha) * 57.295779513082322 - eqt;

    // convert from UTC back to our timezone
    minutes += m_tz;

    // adjust the time array by minutes
    when[HOUR]   = 0;
    when[MINUTE] = 0;
    when[SECOND] = 0;

    Adjust(when, minutes);

    return true;
}

long TKeeper::DayNumber(uint16_t y, uint8_t m, uint8_t d){

    m = (m + 9) % 12;
    y = y - m/10;
    return 365*y + y/4 - y/100 + y/400 + (m*306 + 5)/10 + d - 1 ;
}
