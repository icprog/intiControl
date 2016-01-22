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

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <string.h>

#include "descriptors.h"
#include "Config/AppConfig.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>



class Usb
{
public:
    Usb();

    bool tick();
    bool attached();

private:
    uint8_t m_buffer[GENERIC_REPORT_SIZE];
    USB_ClassInfo_HID_Device_t m_interface;

    bool m_attached;
};
