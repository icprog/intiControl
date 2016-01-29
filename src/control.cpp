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
    if (!time.hour() && !time.minute())
        calcTimes(time);

    if (hitSunrise(time))
    {
        m_led.tick();

        if (!m_seconds--)
            m_led.inverse();
    }
}
void Control::calcTimes(const DateTime &time)
{
    TKeeper::location loc;
    TKeeper tkeep(loc);

    m_sunrise = time;
    m_sunset  = time;

    tkeep.SunRise(m_sunrise);
    tkeep.SunSet (m_sunset);

    m_seconds = m_sunset - m_sunrise;
    m_seconds /= 2;

    for (uint8_t i = 0; i < Led::getMaxCh(); i++)
    {
        Led::Config config = m_led.getConfig(i);

        // work out what the required step size is
        config.step = (config.max - config.min) / m_seconds;

        m_led.setConfig(i, config);
    }
}
bool Control::hitSunrise(const DateTime & time)
{
    return (time.get() >= m_sunrise);
}
bool Control::hitSunset(const DateTime & time)
{
    return (time.get() >= m_sunset);
}
msgStatus Control::getCurrent(const uint32_t &time)
{
    msgStatus ret;

    for (uint8_t i = 0; i < Led::getMaxCh(); i++)
        ret.m_currValues[i] = m_led.getConfig(i).value;

    ret.m_time = time;

    return ret;
}
void Control::setMax(const msgSetMax *setMax)
{
    for (uint8_t i = 0; i < Led::getMaxCh(); i++)
    {
        Led::Config config = m_led.getConfig(i);

        // work out what the required step size is
        config.max = setMax->m_maxValues[i];

        m_led.setConfig(i, config);
    }
}
