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
 * Rtc class derived from Arduino RTC library.
 * https://github.com/jcw/rtclib
 *
 */

// Original Library obtained from --
// A library for handling real-time clocks, dates, etc.
// 2010-02-04 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php
// 2012-11-08 RAM methods - idreammicro.com
// 2012-11-14 SQW/OUT methods - idreammicro.com

#include <avr/interrupt.h>
#include <i2cmaster.h>

#include <rtc.h>

volatile bool ticked = false;

ISR(PCINT1_vect)
{
    ticked = PINC & (1 << PC2);
}

Rtc::Rtc()
{
    i2c_init  ();
    setCharger(2);
    setIRQ    (1);

    DDRC   &= ~(1 << PC2);
    PORTC  |=  (1 << PC2);

    PCMSK1 |= (1 << PCINT11);
    PCICR  |= (1 << PCIE1);
}
Rtc::~Rtc()
{}

bool Rtc::tick()
{
    bool ret = ticked;
    ticked = false;

    return ret;
}

void Rtc::adjust(const DateTime& dt)
{
    if (i2c_init())
    {
        if (i2c_start(BQ32000_ADDRESS | WRITE) && i2c_write(0))
        {
            i2c_write(bin2bcd(dt.second()));
            i2c_write(bin2bcd(dt.minute()));
            i2c_write(bin2bcd(dt.hour()));
            i2c_write(bin2bcd(0));
            i2c_write(bin2bcd(dt.day()));
            i2c_write(bin2bcd(dt.month()));
            i2c_write(bin2bcd(dt.shortyear()));
            i2c_stop();
        }
    }
}
DateTime Rtc::now()
{
    uint8_t  ss = 0;
    uint8_t  mm = 0;
    uint8_t  hh = 0;
    uint8_t  d  = 0;
    uint8_t  m  = 0;
    uint16_t y  = 0;

    if (i2c_init())
    {
        if (i2c_start(BQ32000_ADDRESS | WRITE) && i2c_write(0))
        {
            if (i2c_rep_start(BQ32000_ADDRESS | READ))
            {
                ss = Rtc::bcd2bin(i2c_read(false));
                mm = Rtc::bcd2bin(i2c_read(false)) & 0x7F;
                hh = Rtc::bcd2bin(i2c_read(false));
                d  = Rtc::bcd2bin(i2c_read(false));
                d  = Rtc::bcd2bin(i2c_read(false));
                m  = Rtc::bcd2bin(i2c_read(false));
                y  = Rtc::bcd2bin(i2c_read()) + 2000;

                i2c_stop();
            }
        }
    }

    return DateTime(y, m, d, hh, mm, ss);
}

void Rtc::setIRQ(uint8_t state)
{
    // Set IRQ square wave output state: 0=disabled, 1=1Hz, 2=512Hz.
    if (state)
    {
        // Setting the frequency is a bit complicated on the BQ32000:
        if (i2c_start(BQ32000_ADDRESS | WRITE))
        {
            i2c_write(BQ32000_SFKEY1);
            i2c_write(BQ32000_SFKEY1_VAL);
            i2c_write(BQ32000_SFKEY2_VAL);
            i2c_write((state == 1) ? BQ32000_FTF_1HZ : BQ32000_FTF_512HZ);
            i2c_stop();
        }
    }

    uint8_t value = readRegister(BQ32000_CAL_CFG1);
    value = (!state) ? value & ~(1<<BQ32000__FT) : value | (1<<BQ32000__FT);
    writeRegister(BQ32000_CAL_CFG1, value);
}

void Rtc::setIRQLevel(uint8_t level)
{
    // The IRQ active level bit is in the same register as the calibration
    // settings, so we preserve its current state:
    uint8_t value = readRegister(BQ32000_CAL_CFG1);
    value = (!level) ? value & ~(1<<BQ32000__OUT) : value | (1<<BQ32000__OUT);
    writeRegister(BQ32000_CAL_CFG1, value);
}

void Rtc::setCalibration(int8_t value)
{
    // Sets the calibration value to given value in the range -31 - 31, which
    // corresponds to -126ppm - +63ppm; see table 13 in th BQ32000 datasheet.
    if (value > 31) value = 31;
    if (value < -31) value = -31;
    uint8_t val = (uint8_t) (value < 0) ? -value | (1<<BQ32000__CAL_S) : value;
    val |= readRegister(BQ32000_CAL_CFG1) & ~0x3f;
    writeRegister(BQ32000_CAL_CFG1, val);
}

void Rtc::setCharger(int state)
{
    /* If using a super capacitor instead of a battery for backup power, use this
     * method to set the state of the trickle charger: 0=disabled, 1=low-voltage
     * charge, 2=high-voltage charge. In low-voltage charge mode, the super cap is
     * charged through a diode with a voltage drop of about 0.5V, so it will charge
     * up to VCC-0.5V. In high-voltage charge mode the diode is bypassed and the super
     * cap will be charged up to VCC (make sure the charge voltage does not exceed your
     * super cap's voltage rating!!).
     */
    // First disable charger regardless of state (prevents it from
    // possible starting up in the high voltage mode when the low
    // voltage mode is requested):
    uint8_t value;
    writeRegister(BQ32000_TCH2, 0);
    if (state <= 0 || state > 2) return;
    value = BQ32000_CHARGE_ENABLE;
    if (state == 2) {
        // High voltage charge enable:
        value |= (1 << BQ32000__TCFE);
    }
    writeRegister(BQ32000_CFG2, value);
    // Now enable charger:
    writeRegister(BQ32000_TCH2, 1 << BQ32000__TCH2_BIT);
}


uint8_t Rtc::readRegister(uint8_t address)
{
    uint8_t ret = 0;

    if (i2c_start(BQ32000_ADDRESS | WRITE))
    {
        i2c_write(address);
        i2c_rep_start(BQ32000_ADDRESS | READ);

        ret = i2c_read(true);
        i2c_stop();
    }

    return ret;
}

bool Rtc::writeRegister(uint8_t address, uint8_t * values, uint8_t len)
{
    bool ret = false;

    if (i2c_start(BQ32000_ADDRESS | WRITE))
    {
        uint8_t written = 0;
        i2c_write(address);
        while (written < len && i2c_write(*values++))
            written++;

        i2c_stop();

        ret = (written == len);
    }

    return ret;
}
bool Rtc::writeRegister(uint8_t address, uint8_t value)
{
    bool ret = false;

    if (i2c_start(BQ32000_ADDRESS | WRITE))
    {
        i2c_write(address);
        ret = i2c_write(value);
        i2c_stop();
    }

    return ret;
}

uint8_t Rtc::isrunning()
{
    return !(readRegister(0x0)>>7);
}
