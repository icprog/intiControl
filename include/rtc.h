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
 * Original file built from https://github.com/jcw/rtclib
 *
 */

#pragma once

#include <inttypes.h>
#include <datetime.h>

class Rtc
{
public:
    Rtc();
    ~Rtc();
    static void adjust(const DateTime& dt);
    static DateTime now();

    uint8_t isrunning();

    // Set IRQ output state: 0=disabled, 1=1Hz, 2=512Hz.
    void setIRQ(uint8_t state);

    // Set IRQ output active state to LOW or HIGH.
    void setIRQLevel(uint8_t level);

    // Sets the calibration value to given value in the range -31 - 31, which
    // corresponds to -126ppm - +63ppm; see table 13 in th BQ32000 datasheet.
    void setCalibration(int8_t value);
    void setCharger(int state);

    bool tick();

private:
    uint8_t readRegister(uint8_t address);
    bool writeRegister(uint8_t address, uint8_t value);
    bool writeRegister(uint8_t address, uint8_t * values, uint8_t len);

    static uint8_t bcd2bin (uint8_t val)
    {
        return ((val >> 4) & 0xF) * 10 + ((val) & 0xF);
    }
    static uint8_t bin2bcd (uint8_t val)
    {
        return  ((val / 10) << 4) | (val % 10);
    }

    static const uint8_t BQ32000_ADDRESS         = 0xD0;
    static const uint8_t READ                    = 0x01;
    static const uint8_t WRITE                   = 0x00;

    // BQ32000 register addresses:
    static const uint8_t BQ32000_CAL_CFG1        = 0x07;
    static const uint8_t BQ32000_TCH2            = 0x08;
    static const uint8_t BQ32000_CFG2            = 0x09;
    static const uint8_t BQ32000_SFKEY1          = 0x20;
    static const uint8_t BQ32000_SFKEY2          = 0x21;
    static const uint8_t BQ32000_SFR             = 0x22;

    // BQ32000 config bits:
    static const uint8_t BQ32000__OUT            = 0x07; // CAL_CFG1 - IRQ active state
    static const uint8_t BQ32000__FT             = 0x06; // CAL_CFG1 - IRQ square wave enable
    static const uint8_t BQ32000__CAL_S          = 0x05; // CAL_CFG1 - Calibration sign
    static const uint8_t BQ32000__TCH2_BIT       = 0x05; // TCH2 - Trickle charger switch 2
    static const uint8_t BQ32000__TCFE           = 0x06; // CFG2 - Trickle FET control

    // BQ32000 config values:
    static const uint8_t BQ32000_CHARGE_ENABLE   = 0x05; // CFG2 - Trickle charger switch 1 enable
    static const uint8_t BQ32000_SFKEY1_VAL      = 0x5E;
    static const uint8_t BQ32000_SFKEY2_VAL      = 0xC7;
    static const uint8_t BQ32000_FTF_1HZ         = 0x01;
    static const uint8_t BQ32000_FTF_512HZ       = 0x00;
};
