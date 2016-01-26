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

#include <inttypes.h>
#include <dimmer.h>
#include <settings.h>

class Led
{
public:
    class Config
    {
    public:
        Config()
            : step(1), min(0), max(4096), dimmer(0)
        {}

        float    step;    // how much to step every time tick is called
        float    value;   // running value
        uint16_t min;
        uint16_t max;
        Dimmer * dimmer;
    };

    Led(const Settings::Emitters & settings);

    bool   setConfig(uint8_t ch, float step, uint16_t min, uint16_t max);
    bool   setConfig(uint8_t ch, const Config & config);
    const Config &getConfig(uint8_t ch);

    void inverse();

    void tick();

    static uint8_t getMaxCh()
    {
        return TOTALCH;
    }

    enum Channel
    {
        WHITE      = 0,
        ROYAL_BLUE = 1,
        BLUE       = 2,
        RED        = 3,
        GREEN      = 4,
        VIOLET     = 5,
        YELLOW     = 6,
        TOTALCH    = 7
    };

private:
    // need to be seperate (no container)
    // due to avr-gcc limitation(s)
    // fair enough as embedded, declare all memory
    // upfront
    Dimmer m_white;
    Dimmer m_rblue;
    Dimmer m_blue;
    Dimmer m_red;
    Dimmer m_green;
    Dimmer m_violet;
    Dimmer m_yellow;

    // configuration accross all channels
    Config m_config[TOTALCH];
};

