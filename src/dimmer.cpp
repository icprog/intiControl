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

#include <delays.h>
#include <dimmer.h>
#include <i2cmaster.h>

Dimmer::Dimmer(uint8_t channel, uint16_t max)
    : m_channel(channel),
      m_offset (channel * 256),
      ON_L     (channel * 4 + LED_ON_L),
      ON_H     (channel * 4 + LED_ON_H),
      OFF_L    (channel * 4 + LED_OFF_L),
      OFF_H    (channel * 4 + LED_OFF_H),
      m_value  (0)
{
    i2c_init();

    uint8_t mode1 = MODE1_AUTOINC;
    uint8_t mode2 = MODE2_INVERT;

    write(MODE1, &mode1);
    write(MODE2, &mode2);

    setLevel(max);
}
Dimmer::~Dimmer()
{}
bool Dimmer::setLevel(uint16_t value)
{
    // our ON values are always offset depending on channel
    // this ensures that all channels at least turn ON at different
    // times, reducing inrush requirements
    uint16_t  offset = (value + m_offset) % 4096;
    uint8_t * poff   = (uint8_t*)&offset;
    uint8_t * pon    = (uint8_t*)&m_offset;
    uint8_t   pwm[4] = {pon[0], pon[1], poff[0], poff[1]};

    bool ret = write(ON_L, pwm, 4);

    if (ret)
        m_value = value;

    return ret;
}

void Dimmer::setFrequency(float freq)
{
    float prescaleval = 25000000;

    prescaleval /= 4096;
    prescaleval /= freq;
    prescaleval -= 1;

    uint8_t prescale = floor(prescaleval + 0.5);
    uint8_t oldmode  = read(MODE1) | 0xA1;
    uint8_t newmode  = (oldmode & 0x7F) | 0x10; // sleep

    write(MODE1, &newmode);             // go to sleep
    write(PCA9685_PRESCALE, &prescale); // set the prescaler
    write(MODE1, &oldmode);

    delay_ms(1);

    //write(MODE1, oldmode | 0xa1);
}

uint8_t Dimmer::read(uint8_t addr)
{
    uint8_t ret = 0;

    if (i2c_start(PCA9685 | WRITE))
    {
        i2c_write(addr);
        i2c_rep_start(PCA9685 | READ);

        ret = i2c_read(true);
        i2c_stop();
    }

    return ret;
}

bool Dimmer::write(uint8_t addr, const uint8_t *values, uint8_t len)
{
    bool ret = false;

    if (i2c_start(PCA9685 | WRITE))
    {
        uint8_t written = 0;
        i2c_write(addr);
        while (written < len && i2c_write(*values++))
            written++;

        i2c_stop();

        ret = (written == len);
    }

    return ret;
}
