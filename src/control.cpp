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

#include <control.h>
#include <tkeeper.h>

Control::Control(const Settings::Emitters & led, const DateTime & now)
    : m_sunrise(now), m_sunset(now), m_led(led)
{}

void Control::tick(const DateTime &time)
{
    m_led.tick();

    if (!time.hour() && !time.minute())
        calcTimes();
}
void Control::calcTimes()
{
    // call this once at midnight

    // work out when sunrise and sunset are
    // work out mid point
    // let each led unit
}
bool Control::hitSunrise(const DateTime & time)
{
    bool ret = ((long)time.get() >= (long)m_sunrise.get());

    return ret;
}
bool Control::hitSunset(const DateTime & time)
{
    bool ret = ((long)time.get() >= (long)m_sunset.get());

    return ret;
}
