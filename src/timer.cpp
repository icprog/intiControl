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

#include <avr/interrupt.h>
#include <timer.h>

volatile bool tick;

ISR(TIMER0_OVF_vect)
{
    TCNT0 = 0x05;
    tick  = true;
}

Timer::Timer()
    : m_tick(0.008f)
{
    // set to 8Mhz / 8 == 1Mhz
    tick   = false;
    TCCR0A = 0x00;
    TCCR0B = 0x04;
    TCNT0  = 0x05;
    TIMSK0 = 0x01;
}

bool Timer::ticked()
{
    return tick;
}
void Timer::reset()
{
    tick = false;
}
float Timer::getTick()
{
    return m_tick;
}
