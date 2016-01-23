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

#include <math.h>
#include <led.h>
#include <settings.h>

Led::Led(const Settings::Emitters &settings)
  : m_white (WHITE),
    m_rblue (ROYAL_BLUE),
    m_blue  (BLUE),
    m_red   (RED),
    m_green (GREEN),
    m_violet(VIOLET),
    m_yellow(YELLOW)
{
    m_config[WHITE]     .dimmer = &m_white;
    m_config[WHITE]     .max    = settings.maxWhite;

    m_config[ROYAL_BLUE].dimmer = &m_rblue;
    m_config[ROYAL_BLUE].max    = settings.maxRoyalBlue;

    m_config[BLUE]      .dimmer = &m_blue;
    m_config[BLUE]      .max    = settings.maxBlue;

    m_config[RED]       .dimmer = &m_red;
    m_config[RED]       .max    = settings.maxRed;

    m_config[GREEN]     .dimmer = &m_green;
    m_config[GREEN]     .max    = settings.maxGreen;

    m_config[VIOLET]    .dimmer = &m_violet;
    m_config[VIOLET]    .max    = settings.maxViolet;

    m_config[YELLOW]    .dimmer = &m_yellow;
    m_config[YELLOW]    .max    = settings.maxYellow;
}

bool Led::setConfig(uint8_t ch, float step, uint16_t min, uint16_t max)
{
    bool ret = false;

    if (ch < TOTALCH)
    {
        m_config[ch].step = step;
        m_config[ch].min  = min;
        m_config[ch].max  = max;
    }

    return ret;
}

void Led::tick()
{
    // expect to be called at 1Hz
    Config * config = m_config;
    for (uint8_t i = 0; i < TOTALCH; i++, config++)
    {
        config->value += config->step;

        if (config->value > config->max)
            config->value = config->max;
        if (config->value < config->min)
            config->value = config->min;

        config->dimmer->setLevel((uint16_t)config->value);
    }
}
