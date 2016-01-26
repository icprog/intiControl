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

#include <usb.h>
#include <avr/wdt.h>
#include <avr/power.h>

#include "descriptors.h"

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

uint8_t * buffer;
USB_ClassInfo_HID_Device_t * interface;

bool * usbAttached;
bool * usbData;

//USB_ClassInfo_HID_Device_t Generic_HID_Interface;

void EVENT_USB_Device_Connect(void)
{
    *usbAttached = true;
}

void EVENT_USB_Device_Disconnect(void)
{
    *usbAttached = false;
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    ConfigSuccess &= HID_Device_ConfigureEndpoints(interface);

    USB_Device_EnableSOFEvents();
}

void EVENT_USB_Device_ControlRequest(void)
{
    HID_Device_ProcessControlRequest(interface);
}

void EVENT_USB_Device_StartOfFrame(void)
{
    HID_Device_MillisecondElapsed(interface);
}

uint8_t hack[4];
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
    return true;
}

void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
    buffer  = (uint8_t*)ReportData;

    *usbData = true;
}


Usb::Usb()
    : m_attached(false), m_data(false)
{
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    /* Hardware Initialization */
    USB_Init();

    m_interface.Config.InterfaceNumber          = INTERFACE_ID_GenericHID;
    m_interface.Config.ReportINEndpoint.Address = GENERIC_IN_EPADDR;
    m_interface.Config.ReportINEndpoint.Size    = GENERIC_EPSIZE;
    m_interface.Config.ReportINEndpoint.Banks   = 1;
    m_interface.Config.PrevReportINBuffer       = m_buffer;
    m_interface.Config.PrevReportINBufferSize   = sizeof(m_buffer);

    interface    = &m_interface;
    usbAttached  = &m_attached;
    usbData      = &m_data;
    buffer       = m_buffer;
}

bool Usb::tick()
{
    HID_Device_USBTask(interface);
    USB_USBTask();

    return m_data;
}

bool Usb::attached()
{
    return m_attached;
}
const Message * Usb::read()
{
    m_data = false;
    return (Message*)buffer;
}
bool Usb::send(void * data, uint16_t len)
{
    bool ret = CALLBACK_HID_Device_CreateHIDReport(
                &m_interface,
                0,
                HID_REPORT_ITEM_In,
                data,
                &len);

    return ret;
}
