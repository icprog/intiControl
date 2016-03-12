/*
 *
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

#include <string.h>
#include <usb.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#include "descriptors.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

#include <datetime.h>

#define TIMER_COUNT 0xD1

ISR(TIMER0_OVF_vect)
{
    USB_USBTask();

    TCNT0 = TIMER_COUNT;
};

// dummy values used for testing only
static uint32_t time       = 0;
static uint16_t current[7] = {0, 0, 0, 0, 0, 0, 0};
static uint16_t maximum[7] = {0, 0, 0, 0, 0, 0, 0};
static uint8_t  mode       = 0;

void process_newTime()
{
    // mark command as "accepted" by the application, so that LUFA does not process it
    Endpoint_ClearSETUP();
    Endpoint_ClearStatusStage();


    //timeSet((USB_ControlRequest.wIndex << 16) | USB_ControlRequest.wValue);
    time = ((uint32_t)USB_ControlRequest.wIndex << 16) | USB_ControlRequest.wValue;
}

void process_newCurrent()
{
    // mark command as "accepted" by the application, so that LUFA does not process it
    Endpoint_ClearSETUP();
    Endpoint_ClearStatusStage();


    current[USB_ControlRequest.wIndex] = USB_ControlRequest.wValue;
}

void process_newMaximum()
{
    // mark command as "accepted" by the application, so that LUFA does not process it
    Endpoint_ClearSETUP();
    Endpoint_ClearStatusStage();


    maximum[USB_ControlRequest.wIndex] = USB_ControlRequest.wValue;
}

void process_newMode()
{
    // mark command as "accepted" by the application, so that LUFA does not process it
    Endpoint_ClearSETUP();
    Endpoint_ClearStatusStage();


    mode = USB_ControlRequest.wValue;
}

void process_getTime()
{
    Endpoint_ClearSETUP();
    Endpoint_Write_16_LE(time);
    Endpoint_ClearIN();

    Endpoint_ClearStatusStage();
}
void process_getCurrent()
{
    Endpoint_ClearSETUP();
    Endpoint_Write_Stream_LE(current, 14, 0);
    Endpoint_ClearIN();
    Endpoint_ClearStatusStage();
}
void process_getMaximum()
{
    Endpoint_ClearSETUP();
    Endpoint_Write_Stream_LE(maximum, 14, 0);
    Endpoint_ClearIN();
    Endpoint_ClearStatusStage();
}
void process_getMode()
{
    Endpoint_ClearSETUP();
    Endpoint_Write_8(mode);
    Endpoint_ClearIN();
    Endpoint_ClearStatusStage();
}

void EVENT_USB_Device_ControlRequest()
{
    /* Handle HID Class specific requests */
    switch (USB_ControlRequest.bRequest)
    {
        case HID_REQ_GetReport:
            if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
            {
                switch(USB_ControlRequest.bRequest)
                {
                case SET_TIME:
                    process_newTime();
                    break;

                case SET_CURRENT:
                    process_newCurrent();
                    break;

                case SET_MAX:
                    process_newMaximum();
                    break;

                case SET_MODE:
                    process_newMode();
                    break;
                }
            }

            break;
        case HID_REQ_SetReport:
            if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
            {
                switch(USB_ControlRequest.bRequest)
                {
                case GET_TIME:
                    process_getTime();
                    break;

                case GET_CURRENT:
                    process_getCurrent();
                    break;

                case GET_MAX:
                    process_getMaximum();
                    break;

                case GET_MODE:
                    process_getMode();
                    break;
                }
            }

            break;
    }
}

Usb::Usb()
{
    // setup a local ISR
    TCCR0A  = 0x00;           // normal count up timer operation
    TCCR0B  = 0x05;           // div-1024 = 7812.5Hz clock
    TCNT0   = TIMER_COUNT;    // Interrupt every 10ms
    TIMSK0 |= (1 << TOIE0);   // enable interrupt

    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    /* Hardware Initialization */
    USB_Init();

    GlobalInterruptEnable();
}
