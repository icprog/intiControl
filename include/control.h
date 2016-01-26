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

#include <datetime.h>
#include <settings.h>
#include <led.h>
#include <message.h>

class Control
{
public:
    Control(const Settings::Emitters &led, const DateTime &now);

    void tick(const DateTime & time);

    Status getCurrent(long time);
    void setMax(const SetMax * setMax);

private:
    void calcTimes(const DateTime &time);

    bool hitSunrise(const DateTime & time);
    bool hitSunset (const DateTime & time);

    DateTime m_sunrise;    // time of sunrise
    DateTime m_sunset;     // time of sunset
    int16_t  m_seconds;    // number of seconds to peak (midday)

    Led m_led;
};
