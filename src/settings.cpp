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
 */

#include <settings.h>

Settings::Settings()
{
    m_eeprom.readBlock(EMITTER_OFFSET, m_emitters);
    m_eeprom.readBlock(LCD_OFFSET, m_lcd);
    m_eeprom.readBlock(LCD_TO_OFFSET, m_lcdTimeout);
}
Settings::~Settings()
{}
bool Settings::setEmitters(const Emitters& values)
{
    bool ret = false;
    uint8_t size = m_eeprom.writeBlock(EMITTER_OFFSET, values);

    if (size == sizeof(m_emitters))
    {
        m_emitters = values;
        ret = true;
    }

    return ret;
}
const Settings::Emitters & Settings::getEmitters()
{
    return m_emitters;
}

bool Settings::setLcd(const Lcd& values)
{
    bool ret = false;
    uint8_t size = m_eeprom.writeBlock(LCD_OFFSET, values);

    if (size == sizeof(m_lcd))
    {
        m_lcd = values;
        ret = true;
    }

    return ret;
}
const Settings::Lcd & Settings::getLcd()
{
    return m_lcd;
}
bool Settings::setLcdTimeout(uint8_t timeout)
{
    bool ret = false;
    uint8_t size = m_eeprom.writeBlock(LCD_TO_OFFSET, timeout);

    if (size == sizeof(m_lcdTimeout))
    {
        m_lcdTimeout = timeout;
        ret = true;
    }

    return ret;
}
const uint8_t & Settings::getLcdTimeout()
{
    return m_lcdTimeout;
}
