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

#pragma once

#include <eeprom.h>

/**
 * @brief The Settings class handles the reading and writting of configuration data
 * to the onboard EEPROM.
 */
class Settings
{
public:
    struct Emitters
    {
        uint16_t maxTotal;
        uint16_t maxWhite;
        uint16_t maxRoyalBlue;
        uint16_t maxBlue;
        uint16_t maxRed;
        uint16_t maxGreen;
        uint16_t maxViolet;
        uint16_t maxYellow;
    };

    struct Lcd
    {
        uint16_t maxRed;
        uint16_t maxGreen;
        uint16_t maxBlue;
    };

    Settings();
    ~Settings();

    bool setEmitters(const Emitters &values);
    const Emitters & getEmitters();

    bool setLcd(const Lcd &values);
    const Lcd & getLcd();

    bool setLcdTimeout(uint8_t timeout);
    const uint8_t & getLcdTimeout();

private:
    Emitters m_emitters;
    Lcd      m_lcd;
    uint8_t  m_lcdTimeout;

    static const uint8_t EMITTER_OFFSET = 0;
    static const uint8_t LCD_OFFSET     = 16;
    static const uint8_t LCD_TO_OFFSET  = 22;

    Eeprom m_eeprom;
};
